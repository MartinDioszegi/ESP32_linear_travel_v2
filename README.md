# ESP32_linear_travel_v2


pohon dvou kamer na lineárním vedení HIVIN xxxxx.xxxxx pomocí krokových motorů NEMA17 (creality 42-40).
Motory jsou řízeny drivery A4988 v režimu osminy kroku. Jako řídící člen je použita vývojová deska ESP-32S 30PIN DEVBOARD.
Délka pojezdu je 2m, koncové spínače nebyly realizovány. Kamery mohou bourat do koncových poloh i do sebe navzájem.

![IMG_20210807_165539](https://user-images.githubusercontent.com/53040547/130255840-9f4e6890-bccf-4933-99f1-3af9c84fe13d.jpg)

NAPÁJENÍ:
Pro napájení je použit zdroj AXIMA AXSP03P01 230VAC/24VDC/1,3A. Vstupní napětí 230VAC je vypínané hlavním vypínačem linky.
Výstup zdroje je jištěn pojistkou 1A v pojistkovém odpínači.
Napětí 24VDC napájí ovládací otočné přepínače umístěné na hlavním operátorském pultu a zároveň BOX s elektronikou pro pojezdy.
V BOXu je 24VDC použito k napájení silové části driverů a napájení step-down měniče 24VDC/5,2VDC 2A s LM2596+LED.
Mikrokontrolér je napájen stabilizátorem napětí 3,3V s AMS1117.
[SLL_EDB_CAM-B_pojezd_kamery_nože.pdf](https://github.com/panMasinka/ESP32_linear_travel_v2/files/7022631/SLL_EDB_CAM-B_pojezd_kamery_noze.pdf)


DRIVER:
![A4988-Stepper-Driver1](https://user-images.githubusercontent.com/53040547/130250913-9817a3c7-f182-4ff4-b56b-21a685e2d233.jpg)



![A4988-Specsifications](https://user-images.githubusercontent.com/53040547/129195844-558fbcbb-ea0c-4d48-9e05-0b6315cee30a.png)


A4988 microstepping . . . . oproti schématu je potřeba propojit piny MS1 a MS2 s ovládacím napětím 3.3V

![A4988-Truth-Table1-300x227](https://user-images.githubusercontent.com/53040547/130251128-a9126449-7a1e-4779-8c4f-b232dab754f4.png)


Na desce pojezd kamer okrajků je propojení pinů MS1, MS2 a Vcc řešeno přímo na driveru
![IMG_20210625_210154](https://user-images.githubusercontent.com/53040547/130251799-0bc363d6-4420-4acf-bf98-9ba6ddc9ac32.jpg)

Na náhradní desce je to řešeno přímo na PCB ze spodní strany.
![IMG_20210813_171620](https://user-images.githubusercontent.com/53040547/130251877-6bb07aa9-28f8-46dc-b9a1-f1e012a12a42.jpg)


NASTAVENÍ PRACOVNÍHO PROUDU MOTORŮ:
Proud, který teče do motoru je na driveru A4988 nastaven referenčním napětím měřitelným mezi trimerem a nulou ovládacího napětí.
![IMG_20210813_165621](https://user-images.githubusercontent.com/53040547/130252084-9be340fd-b79a-4346-9255-96255d503b58.jpg)




