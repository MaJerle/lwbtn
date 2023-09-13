# Changelog

## Develop

- Add `lwbtn_keepalive_get_period` function

## v1.0.0

- Send `CLICK` event if there is an overlap between max time between clicks and new click arrives
- Do not send `CLICK` event if there was previously detected long hold press (hold time exceeded max allowed click time)

## v0.0.2

- Add `LWBTN_CFG_GET_STATE_MODE` to control *get state* mode
- Add option to check if button is currently active (after debounce period has elapsed) 
- Add option to set time/click parameters at run time for each button specifically
- Rename `_RUNTIME` configuration with `_DYNAMIC` 
- Change `LWBTC_CFG_TIME_DEBOUNCE` to `LWBTC_CFG_TIME_DEBOUNCE_PRESS` and `LWBTC_CFG_TIME_DEBOUNCE_RUNTIME` to `LWBTC_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC` respectively
- Add option release debounce with `LWBTC_CFG_TIME_DEBOUNCE_RELEASE` and `LWBTC_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC` options

## v0.0.1

- First commit