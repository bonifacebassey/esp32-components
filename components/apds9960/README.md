# Usage Example

## To start sensing
1. Include the "apds9960_api.h" library
``` c
#include "apds9960_api.h"
```

2. Define apds9960-gesture callback function
``` c
static void apds9960_gesture_event(apds9960_cb_event_t event, apds9960_cb_param_t *param)
{
    switch (event)
    {
        case APDS9960_GESTURE_UP_EVT:
            printf("gesture APDS9960-UP...\n");
            break;
        case APDS9960_GESTURE_DOWN_EVT:
            printf("gesture APDS9960-DOWN...\n");
            break;
        case APDS9960_GESTURE_LEFT_EVT:
            printf("gesture APDS9960-LEFT...\n");
            break;
        case APDS9960_GESTURE_RIGHT_EVT:
            printf("gesture APDS9960-RIGHT...\n");
            break;
        default:
            break;
    }
}
```

3. Register apds9960-gesture callback
``` c
apds9960_register_callback(apds9960_gesture_event);
```

4. Initialize apds9960-gesture
``` c
apds9960_init();
```

## To stop Sensing

1. Unregister apds9960-gesture callback
``` c
void apds9960_unregister_callback();
```

2. Deinitialize apds9960-gesture
``` c
apds9960_deinit();
```