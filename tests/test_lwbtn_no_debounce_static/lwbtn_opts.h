#ifndef LWBTN_HDR_OPTS_H
#define LWBTN_HDR_OPTS_H

/* Disable debounce completely, to test immediate on-press/on-release detection */
#define LWBTN_CFG_TIME_DEBOUNCE_PRESS   0
#define LWBTN_CFG_TIME_DEBOUNCE_RELEASE 0

/* Drive input state manually from the test, instead of through a get-state callback */
#define LWBTN_CFG_GET_STATE_MODE        LWBTN_GET_STATE_MODE_MANUAL

#endif /* LWBTN_HDR_OPTS_H */
