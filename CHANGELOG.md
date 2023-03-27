# Changelog

## Develop

- Add `LWBTN_CFG_GET_STATE_MODE` to control *get state* mode
- Add option to check if button is currently active (after debounce period has elapsed) 
- Add option to set time/click parameters at run time for each button specifically
- Rename `_RUNTIME` configuration with `_DYNAMIC` 
- Change `LWBTC_CFG_TIME_DEBOUNCE` to `LWBTC_CFG_TIME_DEBOUNCE_PRESS` and `LWBTC_CFG_TIME_DEBOUNCE_RUNTIME` to `LWBTC_CFG_TIME_DEBOUNCE_PRESS_DYNAMIC` respectively
- Add option release debounce with `LWBTC_CFG_TIME_DEBOUNCE_RELEASE` and `LWBTC_CFG_TIME_DEBOUNCE_RELEASE_DYNAMIC` options

## v0.0.1

- First commit