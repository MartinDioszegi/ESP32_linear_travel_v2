# ESP32_linear_travel_v2

pohon dvou kamer na lineárním vedení HIVIN xxxxx.xxxxx pomocí krokových motorů NEMA17 (creality 42-40).
Motory jsou řízeny drivery A4988 v režimu osminy kroku.

![A4988-Stepper-Driver1](https://user-images.githubusercontent.com/53040547/130250913-9817a3c7-f182-4ff4-b56b-21a685e2d233.jpg)



![A4988-Specsifications](https://user-images.githubusercontent.com/53040547/129195844-558fbcbb-ea0c-4d48-9e05-0b6315cee30a.png)


A4988 microstepping . . . . oproti schématu je potřeba propojit piny MS1 a MS2 s ovládacím napětím 3.3V

![A4988-Truth-Table1-300x227](https://user-images.githubusercontent.com/53040547/130251128-a9126449-7a1e-4779-8c4f-b232dab754f4.png)


Na desce pojezd kamer okrajků je propojení pinů MS1, MS2 a Vcc řešeno přímo na driveru



