#ifndef LWBTN_HDR_OPTS_H
#define LWBTN_HDR_OPTS_H

/* Disable debounce completely, to isolate the keep-alive timing under test */
#define LWBTN_CFG_TIME_DEBOUNCE_PRESS           0
#define LWBTN_CFG_TIME_DEBOUNCE_RELEASE         0

/* Keep alive period must be runtime-configurable, so the test can set it to 0 */
#define LWBTN_CFG_TIME_KEEPALIVE_PERIOD_DYNAMIC 1

/* Drive input state manually from the test, instead of through a get-state callback */
#define LWBTN_CFG_GET_STATE_MODE                LWBTN_GET_STATE_MODE_MANUAL

#endif /* LWBTN_HDR_OPTS_H */
