How to use the input devices:
-----------------------------

The gamepad, keyboard and mouse input does behave on Android, Linux and Windows the same.
This allows the same input behaviour on all devices. 

Gamepad:   
![Gamepad](images/gamepad_layout.png)  
Two gamepads are supported and the first two devices found are used.
The axis values are adjusted, that the returned values are the same on all OSes.


Keyboard:   
![Keyboard](images/keyboard_layout.png)  
On all OSes and lanuguage settings, the above keyboard layout is used.
So, a key input depends on the actual physical key position and not its letter.
If a key is not named in the above image (except space), the key can not be queried.


Mouse:  
Left, middle and right buttons are supported. Also, the mouse wheel can be queried.
