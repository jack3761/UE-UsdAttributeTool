# Unreal Engine USD Attribute Tools plugin

## Overview

This plugin is split into three sections. The "UsdCameraFrameRanges" module is an editor module that is opened from the window tab, and displays the camera information from the cameras found within the found USD file. Additionally it provides access to USD attributes, an automatic material swap based on the Usd material name and an automatic disable manual focus button for all cameras in the level for editor purposes.

The "UsdAttributeFunctionLibrary" module provides a selection of blueprint callable functions to directly access USD attribute values at runtime. This covers integers, floats, doubles and Vec3 for all of those too. This is for both static and time sampled values.

Within the plugin content, there is a selection of button widgets to provide access to the level sequences, or automatically access a Usd sequence. These buttons, and pause/play and stop buttons all use the Widget Button Function library, which is a collection of functions that control the level sequence player. To share the same level sequence player, this calls to a blueprint holding a level sequence player as a variable.

### UsdCameraFrameRanges

### UsdAttributeFunctionLibrary

### Widget Button Function Library
