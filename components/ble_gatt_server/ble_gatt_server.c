#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <nvs_flash.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bt_controller.h"
#include "ble_gatt_server.h"
#include "ble_gatts_table.h"

#define PROFILE_NUM 1
#define PROFILE_APP_IDX 0
#define APPLICATION_ID 0x55
#define SVC_INST_ID 0
#define PREPARE_BUF_MAX_SIZE 1024

#define DEFAULT_CHAR_MTU 20
#define MAXIMUM_CHAR_MTU 500
#define ATT_MTU_3 3
volatile uint16_t char_mtu_size = DEFAULT_CHAR_MTU;

#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)

static const char *TAG = "ble_gattserver";

bool allow_pairing = false;
static volatile bool sending_busy = false;
static volatile bool notify_enabled = false;
static volatile bool is_connected = false;
connection_interval_t conn_interval = FAST;

static uint8_t adv_config_done = 0;
static uint16_t gatts_attr_table[IDX_ATTR_IDX_NUM];
uint8_t receive_buffer[MAXIMUM_CHAR_MTU];

static esp_err_t set_device_name(const char *name);
static esp_err_t set_security_parameters(bool validate);
static void ble_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

typedef struct
{
    uint8_t *prepare_buf;
    int prepare_len;
} prepare_type_env_t;
prepare_type_env_t prepare_write_env;

static uint8_t adv_service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0x55, 0xE4, 0x05, 0xD2, 0xAF, 0x9F, 0xA9, 0x8F, 0xE5, 0x4A, 0x7D, 0x00, 0xBB, 0x00, 0x00, 0x00};

#define MANUFACTURER_DATA_LEN 15
static uint8_t manufacturer_data[MANUFACTURER_DATA_LEN] = {'D', 'O', 'R', 'I', 'T'};

esp_ble_adv_data_t advertisement_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x000C, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = ESP_BLE_APPEARANCE_UNKNOWN,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid),
    .p_service_uuid = adv_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
esp_ble_adv_data_t scan_response_data = {
    .set_scan_rsp = true,
    .manufacturer_len = MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = &manufacturer_data[0],
};

// configuration parameters required by the BLE stack to execute
esp_ble_adv_params_t advertisement_param = {

    .adv_int_min = 0x0020,
    .adv_int_max = 0x0040,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    esp_bd_addr_t client_address;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

// This is our gatts application profile
// One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT
static struct gatts_profile_inst profile_table[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

esp_err_t ble_start(const char *name, bool with_pin, power_level_t power_level, connection_interval_t c_interval)
{
    esp_err_t ret = ESP_FAIL;

    do
    {
        conn_interval = c_interval;

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ESP_ERROR_CHECK(nvs_flash_init());
        }

        ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "bt classic mem release failed");
            break;
        }

        ret = bt_controller_start(ESP_BT_MODE_BLE);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "initialize controller failed");
            break;
        }

        ret = ble_set_power_level(ESP_BLE_PWR_TYPE_DEFAULT, power_level);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "set TX power failed");
            break;
        }

        ret = esp_bluedroid_init();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "initialize bluedroid failed");
            break;
        }

        ret = esp_bluedroid_enable();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "ebable bluedroid failed");
            break;
        }

        ret = esp_ble_gatts_register_callback(ble_gatts_callback);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "gatts register failed");
            break;
        }

        ret = esp_ble_gap_register_callback(ble_gap_callback);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "gap register failed");
            break;
        }

        ret = set_device_name(name);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "set device name failed");
            break;
        }

        ret = esp_ble_gatts_app_register(APPLICATION_ID);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "gatts app register failed");
            break;
        }

        ret = esp_ble_gatt_set_local_mtu(MAXIMUM_CHAR_MTU + ATT_MTU_3);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "set local  MTU failed");
            break;
        }

        ret = set_security_parameters(with_pin);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "set security params failed");
            break;
        }

    } while (false);

    return ret;
}

esp_err_t ble_stop()
{
    esp_err_t ret = ESP_FAIL;

    do
    {
        ret = esp_ble_gatts_app_unregister(profile_table[PROFILE_APP_IDX].gatts_if);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "unregister application failed");
            break;
        }

        ret = esp_bluedroid_disable();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "bluedroid disable failed");
            break;
        }

        ret = esp_bluedroid_deinit();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "bluedroid deinit failed");
            break;
        }

        ret = bt_controller_stop();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "stop controller failed");
            break;
        }
    } while (false);

    return ret;
}

esp_err_t ble_start_pairing(const char *name, bool with_pin)
{
    esp_err_t ret = ESP_FAIL;

    do
    {
        allow_pairing = true;

        ret = set_device_name(name);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "set device name failed");
            break;
        }

        esp_ble_io_cap_t iocap = with_pin ? ESP_IO_CAP_IO : ESP_IO_CAP_NONE;
        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "start pairing with PIN SetSecurityParameters failed");
            break;
        }

        ret = esp_ble_gap_config_adv_data(&advertisement_data);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "config advertisement_data failed");
            break;
        }

    } while (false);

    return ret;
}

esp_err_t ble_stop_pairing(const char *name)
{
    esp_err_t ret = ESP_FAIL;

    do
    {
        esp_ble_io_cap_t iocap = ESP_IO_CAP_IO;
        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "stop pairing with failed");
            break;
        }

        allow_pairing = true;

    } while (false);

    return ret;
}

esp_err_t ble_clear_paired_devices()
{
    esp_err_t ret = ESP_FAIL;
    esp_ble_bond_dev_t *dev_list = NULL;

    do
    {
        int dev_num = esp_ble_get_bond_device_num();
        ESP_LOGI(TAG, "paired devices found: %d", dev_num);
        if (!dev_num)
            break;

        dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
        ret = esp_ble_get_bond_device_list(&dev_num, dev_list);
        if (ret != ESP_OK)
            break;

        for (int i = 0; i < dev_num; ++i)
        {
            ret = esp_ble_remove_bond_device(dev_list[i].bd_addr);
            if (ret != ESP_OK)
                break;
        }

    } while (false);

    if (dev_list)
        free(dev_list);

    if (ret != ESP_OK)
        ESP_LOGI(TAG, "clear paired devices success");
    else
        ESP_LOGE(TAG, "clear paired devices failed. result: %d", ret);

    return ret;
}

esp_err_t ble_send(uint8_t *data, size_t size)
{
    do
    {
        while (sending_busy)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        sending_busy = true;

        esp_ble_gatts_set_attr_value(gatts_attr_table[IDX_CHAR_VAL_READ_WRITE], size, data);

        // need-confirm ->
        // yes=indicate (i.e send with ACK) slower
        // no=notify (i.e fire and forget) faster
        esp_ble_gatts_send_indicate(profile_table[PROFILE_APP_IDX].gatts_if, profile_table[PROFILE_APP_IDX].conn_id,
                                    profile_table[PROFILE_APP_IDX].char_handle, sizeof(data), data, false);

        // esp_ble_gatts_send_indicate(MMS_profile_tab[PROFILE_APP_IDXX].gatts_if, connection.Value(),
        //                             MMS_gatts_attr_table[MMS_CHAR_VAL_READ], data.size(), const_cast<std::uint8_t *>(data.data()),
        //                             true);

    } while (false);

    return ESP_OK;
}

esp_err_t ble_receive(esp_gatt_if_t gatts_if, uint8_t handle, uint16_t conn_id, uint32_t trans_id)
{
    esp_gatt_rsp_t rsp;
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = handle;
    rsp.attr_value.len = 4;
    rsp.attr_value.value[0] = 0xde;
    rsp.attr_value.value[1] = 0xed;
    rsp.attr_value.value[2] = 0xbe;
    rsp.attr_value.value[3] = 0xef;
    esp_ble_gatts_send_response(gatts_if, conn_id, trans_id, ESP_GATT_OK, &rsp);

    return ESP_OK;
}

esp_err_t update_connection_parameters(connection_interval_t conn_interval)
{
    esp_ble_conn_update_params_t conn_params;
    memcpy(conn_params.bda, profile_table[PROFILE_APP_IDX].client_address, sizeof(esp_bd_addr_t));
    conn_params.min_int = 0x0006;
    conn_params.max_int = 0x000C;
    conn_params.latency = 0;
    conn_params.timeout = 400; // timeout = timeout * 10ms = 4000ms
    esp_log_buffer_hex("ble_gattserver: update conn_params for", profile_table[PROFILE_APP_IDX].client_address, sizeof(esp_bd_addr_t));

    switch (conn_interval)
    {
    // interval = min|max interval * 1.25ms
    case FAST: // 7 - 15 ms
        conn_params.min_int = 0x0006;
        conn_params.max_int = 0x000C;
        break;

    case FASTER: // 30 - 50 ms
        conn_params.min_int = 0x0018;
        conn_params.max_int = 0x0028;
        break;

    case SLOW: // 100 - 125 ms
        conn_params.min_int = 0x0050;
        conn_params.max_int = 0x0064;
        break;

    case SLOWER: // 3s
        conn_params.min_int = 0x0960;
        conn_params.max_int = 0x0960;
        break;

    default:
        break;
    }

    return esp_ble_gap_update_conn_params(&conn_params);
}
static void ble_show_paired_devices(void)
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    ESP_LOGI(TAG, "paired devices number : %d\n", dev_num);

    ESP_LOGI(TAG, "paired devices list : %d\n", dev_num);
    for (int i = 0; i < dev_num; i++)
    {
        esp_log_buffer_hex(TAG, (void *)dev_list[i].bd_addr, sizeof(esp_bd_addr_t));
    }

    free(dev_list);
}

static esp_err_t set_device_name(const char *name)
{
    esp_err_t ret = ESP_FAIL;

    do
    {
        size_t max_name_length = 32;
        if (strlen(name) > 1 && strlen(name) < max_name_length)
        {
            ESP_LOGE(TAG, "device name is empty or greater than %d characters", max_name_length);
            break;
        }

        ret = esp_ble_gap_set_device_name(name);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "set device name failed");
            break;
        }

        ESP_LOGI(TAG, "device name set to: %s", name);

    } while (false);

    return ret;
}

static esp_err_t set_security_parameters(bool with_pin)
{
    esp_err_t ret = ESP_FAIL;

    do
    {
        esp_ble_io_cap_t iocap = with_pin ? ESP_IO_CAP_IO : ESP_IO_CAP_NONE;
        esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
        uint8_t key_size = 16;
        uint8_t init_key = (ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
        uint8_t rsp_key = (ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
        uint32_t passkey = 987654;
        uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
        uint8_t oob_support = ESP_BLE_OOB_DISABLE;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

        ret = esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
        if (ret != ESP_OK)
            break;

    } while (false);

    return ret;
}

// data is received from client - process it here
static void on_data_received(uint8_t *received_data, uint16_t data_size)
{
    ESP_LOGI(TAG, "data received from client: %d", data_size);
    esp_log_buffer_hex(TAG, received_data, data_size);
    if (data_size < MAXIMUM_CHAR_MTU)
    {
        memset(receive_buffer, 0, sizeof(receive_buffer));
        memcpy(receive_buffer, received_data, data_size);
    }
}

static void ble_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            profile_table[PROFILE_APP_IDX].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGE(TAG, "register application failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == profile_table[idx].gatts_if)
            {
                if (profile_table[idx].gatts_cb)
                {
                    profile_table[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (false);
}

static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&advertisement_param);
        }
        break;

    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&advertisement_param);
        }
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "advertising start failed, error status = %x", param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "advertising start success");
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "advertising stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "advertising stop success");
        break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "update connection params : status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;

    case ESP_GAP_BLE_NC_REQ_EVT:
        if (allow_pairing)
        {
            memcpy(profile_table[PROFILE_APP_IDX].client_address, param->ble_security.ble_req.bd_addr, sizeof(esp_bd_addr_t));
            esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
            ESP_LOGI(TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
        }
        else
        {
            ESP_LOGE(TAG, "reject ESP_GAP_BLE_NC_REQ_EVT. Pairing not activated");
            esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, false);
            esp_ble_gap_disconnect(param->ble_security.ble_req.bd_addr);

            // When pairing cannot be done, subdue over 20 <await async> requests from client 1.
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        break;

    case ESP_GAP_BLE_KEY_EVT:
        // shows the ble key info share with peer device to the user.
        ESP_LOGI(TAG, "ESP_GAP_BLE_KEY_EVT : key type = %d", param->ble_security.ble_key.key_type);
        break;

    case ESP_GAP_BLE_PASSKEY_REQ_EVT:
        if (!allow_pairing)
        {
            ESP_LOGE(TAG, "reject ESP_GAP_BLE_PASSKEY_REQ_EVT. Pairing not activated");
            esp_ble_gap_disconnect(param->ble_security.ble_req.bd_addr);
            break;
        }
        ESP_LOGI(TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        esp_ble_passkey_reply(profile_table[PROFILE_APP_IDX].client_address, true, 0x00); // to input the passkey displayed on the remote device
        break;

    case ESP_GAP_BLE_SEC_REQ_EVT:
        if (!allow_pairing)
        {
            ESP_LOGE(TAG, "reject ESP_GAP_BLE_SEC_REQ_EVT. Pairing not activated");
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, false);
            esp_ble_gap_disconnect(param->ble_security.ble_req.bd_addr);
            break;
        }
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        ESP_LOGI(TAG, "ESP_GAP_BLE_SEC_REQ_EVT");
        break;

    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
        if (!allow_pairing)
        {
            ESP_LOGE(TAG, "reject ESP_GAP_BLE_PASSKEY_NOTIF_EVT. Pairing not activated");
            esp_ble_gap_disconnect(param->ble_security.ble_req.bd_addr);
            break;
        }
        ESP_LOGI(TAG, "The passkey Notify number:%06d", param->ble_security.key_notif.passkey);
        break;

    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        ESP_LOGI(TAG, "remote BD_ADDR:");
        esp_log_buffer_hex(TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(TAG, "pair status = %s", param->ble_security.auth_cmpl.success ? "success" : "fail");
        if (!param->ble_security.auth_cmpl.success)
        {
            ESP_LOGI(TAG, "fail reason = 0x%x", param->ble_security.auth_cmpl.fail_reason);
        }
        else
        {
            ESP_LOGI(TAG, "authentication complete. status = %d", param->ble_security.auth_cmpl.success);
        }
        ble_show_paired_devices();
        break;

    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
        ESP_LOGI(TAG, "-----REMOVE_BOND_DEV, status=%d-----", param->remove_bond_dev_cmpl.status);
        esp_log_buffer_hex(TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "------------------------------------");
        break;

    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:

        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "config local privacy failed, error status = %x", param->local_privacy_cmpl.status);
            break;
        }

        esp_err_t ret = esp_ble_gap_config_adv_data(&advertisement_data);
        if (ret == ESP_OK)
            adv_config_done |= ADV_CONFIG_FLAG;

        ret = esp_ble_gap_config_adv_data(&scan_response_data);
        if (ret == ESP_OK)
            adv_config_done |= ADV_CONFIG_FLAG;

        if (ret != ESP_OK)
            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);

        break;
    default:
        break;
    }
}

void prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(TAG, "prepare write : conn_id = %d, handle = %d, value len = %d, offset = %d",
             param->write.conn_id, param->write.handle, param->write.len, param->write.offset);

    esp_gatt_status_t status = ESP_GATT_OK;

    // Ensure that write data is not larger than max attribute length
    if (param->write.offset > PREPARE_BUF_MAX_SIZE)
    {
        status = ESP_GATT_INVALID_OFFSET;
    }
    else if (param->write.offset + param->write.len > PREPARE_BUF_MAX_SIZE)
    {
        status = ESP_GATT_INVALID_ATTR_LEN;
    }
    else
    {
        // If prepared buffer is not allocated, then allocate it
        if (prepare_write_env->prepare_buf == NULL)
        {
            prepare_write_env->prepare_len = 0;
            prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t *));
            if (prepare_write_env->prepare_buf == NULL)
            {
                ESP_LOGE(TAG, "failed to allocate prepare buf");
                status = ESP_GATT_NO_RESOURCES;
            }
        }
    }
    // Send write response when param->write.need_rsp is true
    if (param->write.need_rsp)
    {
        if (status == ESP_GATT_OK)
        {
            esp_gatt_rsp_t gatt_rsp;
            gatt_rsp.attr_value.len = param->write.len;
            gatt_rsp.attr_value.handle = param->write.handle;
            gatt_rsp.attr_value.offset = param->write.offset;
            gatt_rsp.attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;

            if (gatt_rsp.attr_value.len && param->write.len)
            {
                memcpy(gatt_rsp.attr_value.value, param->write.value, param->write.len);
                if (esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, &gatt_rsp) != ESP_OK)
                    ESP_LOGE(TAG, "Send response error in prep write");
            }
        }
        else
        {
            if (esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL) != ESP_OK)
                ESP_LOGE(TAG, "Send response error in prep write");
        }
    }

    if (status != ESP_GATT_OK)
    {
        if (prepare_write_env->prepare_buf)
        {
            free(prepare_write_env->prepare_buf);
            prepare_write_env->prepare_buf = NULL;
            prepare_write_env->prepare_len = 0;
        }
        return;
    }

    // If prepared buffer is allocated successfully, copy incoming data into it
    if (status == ESP_GATT_OK)
    {
        memcpy(prepare_write_env->prepare_buf + param->write.offset, param->write.value, param->write.len);
        prepare_write_env->prepare_len += param->write.len;
    }
}

void exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf)
    {
        on_data_received(prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }
    else
    {
        ESP_LOGI(TAG, "ESP_GATT_PREP_WRITE_CANCEL");
    }

    if (prepare_write_env->prepare_buf)
    {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        profile_table[PROFILE_APP_IDX].service_id.is_primary = true;
        profile_table[PROFILE_APP_IDX].service_id.id.inst_id = 0x00;
        profile_table[PROFILE_APP_IDX].service_id.id.uuid.len = ESP_UUID_LEN_16;
        profile_table[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid16 = SERVICE_UUID;
        profile_table[PROFILE_APP_IDX].gatts_if = gatts_if;

        esp_ble_gap_config_local_privacy(true);

        if (esp_ble_gatts_create_attr_tab(gatts_attr_database, gatts_if, IDX_ATTR_IDX_NUM, SVC_INST_ID) != ESP_OK)
            ESP_LOGE(TAG, "create_attribute_table failed");
        break;

    case ESP_GATTS_READ_EVT:
        ESP_LOGI(TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        ble_receive(gatts_if, param->read.handle, param->read.conn_id, param->read.trans_id);
        break;

    case ESP_GATTS_WRITE_EVT:
        if (!param->write.is_prep)
        {
            ESP_LOGI(TAG, "GATT_WRITE_EVT, conn_id = %d, handle = %d, value len = %d", param->write.conn_id, param->write.handle, param->write.len);
            esp_log_buffer_hex(TAG, param->write.value, param->write.len);

            if (gatts_attr_table[IDX_CHAR_CFG_READ_WRITE] == param->write.handle && param->write.len == 2)
            {
                uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];

                if (descr_value == 0x0001)
                {
                    notify_enabled = true;
                    ESP_LOGI(TAG, "notify enable (%d)", notify_enabled);
                }
                else if (descr_value == 0x0002)
                {
                    ESP_LOGI(TAG, "indicate enable");
                }
                if (descr_value == 0x0000)
                {
                    notify_enabled = false;
                    ESP_LOGI(TAG, "notify/indicate disable");
                }
                else
                {
                    ESP_LOGE(TAG, "unknown descr value");
                    esp_log_buffer_hex(TAG, param->write.value, param->write.len);
                }
            }
            else
            {
                // data (param->write.value, param->write.len) received from client, process it
                on_data_received(param->write.value, param->write.len);
            }
            // send response when param->write.need_rsp is true
            if (param->write.need_rsp)
            {
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            }
        }
        else
        {
            prepare_write_event_env(gatts_if, &prepare_write_env, param);
        }
        break;
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT");
        exec_write_event_env(&prepare_write_env, param);
        break;

    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
        sending_busy = (param->conf.status == ESP_GATT_OK) ? false : true;
        break;

    case ESP_GATTS_START_EVT:
        ESP_LOGI(TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
        break;

    case ESP_GATTS_CONNECT_EVT:
    {
        char_mtu_size = DEFAULT_CHAR_MTU;
        esp_log_buffer_hex("ble_gattserver: ESP_GATTS_CONNECT_EVT. remote address", param->connect.remote_bda, sizeof(esp_bd_addr_t));
        esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);

        update_connection_parameters(conn_interval);

        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
        esp_log_buffer_hex(TAG, param->connect.remote_bda, ESP_BD_ADDR_LEN);

        profile_table[PROFILE_APP_IDX].conn_id = param->connect.conn_id;
        is_connected = true;
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&advertisement_param);
        is_connected = false;
        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    {
        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "Create attribute table failed, error code = %x", param->create.status);
        }
        else if (param->add_attr_tab.num_handle != IDX_ATTR_IDX_NUM)
        {
            ESP_LOGE(TAG, "create_attr_table abnormally, num_handle (%d) not equal to IDX_ATTR_IDX_NUM(%d)",
                     param->add_attr_tab.num_handle, IDX_ATTR_IDX_NUM);
        }
        else
        {
            ESP_LOGI(TAG, "create attribute table successfully, the number handle = %d\n", param->add_attr_tab.num_handle);
            memcpy(gatts_attr_table, param->add_attr_tab.handles, sizeof(gatts_attr_table));
            esp_ble_gatts_start_service(gatts_attr_table[IDX_SERVICE]);
        }
        break;
    }

    case ESP_GATTS_MTU_EVT:
        char_mtu_size = (param->mtu.mtu > MAXIMUM_CHAR_MTU) ? MAXIMUM_CHAR_MTU : param->mtu.mtu - ATT_MTU_3;
        ESP_LOGI(TAG, "onRequestMTUChange conn_id = %d, mtu_size = %d", param->mtu.conn_id, char_mtu_size);
        break;

    case ESP_GATTS_CONGEST_EVT:
        ESP_LOGW(TAG, "congestion = %d", param->congest.congested);
        break;

    default:
        break;
    }
}