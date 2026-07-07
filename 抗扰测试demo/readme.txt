代码设置一个目标角度，然后开机后自己转到那个设置的角度。手动转动电机后其可以自动转回原来设置的角度。

修改位置是“[ApplyFunction.c (line 22)](/C:/Users/Easyj/Documents/fu6366q电机/静止保持-抗扰动测试demo/User/source/ApplyFunction.c:22)”
#define POSITION_HOLD_TARGET_DEG (30L)       // 默认开机转到30°