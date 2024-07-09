# Board definitions

These are placeholder projects that mostly exist because it makes it easier to build for individual hardware:
* We can have a different (generated) `sdkconfig` per board
* Build cache target files are kept separate per hardware

A project here 
* should only do the bare minimum and hand off to the main `fri3d_firmware` component
* should not implement code, this goes in the respective bsp component
* should never commit a `sdkconfig`, instead define required values in a `sdkconfig.defaults`
