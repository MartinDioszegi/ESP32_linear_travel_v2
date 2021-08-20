# ESP32_linear_travel_v2


pohon dvou kamer na lineárním vedení HIVIN xxxxx.xxxxx pomocí krokových motorů NEMA17 (creality 42-40).
Motory jsou řízeny drivery A4988 v režimu osminy kroku. Jako řídící člen je použita vývojová deska ESP-32S 30PIN DEVBOARD.
Délka pojezdu je 2m, koncové spínače nebyly realizovány. Kamery mohou bourat do koncových poloh i do sebe navzájem.

![image18](https://user-images.githubusercontent.com/53040547/130271699-146104c1-0483-49fb-8483-abf60e550825.png)


**NAPÁJENÍ:**
Pro napájení je použit zdroj AXIMA AXSP03P01 230VAC/24VDC/1,3A. Vstupní napětí 230VAC je vypínané hlavním vypínačem linky.
Výstup zdroje je jištěn pojistkou 1A v pojistkovém odpínači.
Napětí 24VDC napájí ovládací otočné přepínače umístěné na hlavním operátorském pultu a zároveň BOX s elektronikou pro pojezdy.
V BOXu je 24VDC použito k napájení silové části driverů a napájení step-down měniče 24VDC/5,2VDC 2A s LM2596+LED.
Mikrokontrolér je napájen stabilizátorem napětí 3,3V s AMS1117.

**SCHÉMA SPOJENÍ S LINKOU:**
[SLL_EDB_CAM-B_pojezd_kamery_nože.pdf](https://github.com/panMasinka/ESP32_linear_travel_v2/files/7022631/SLL_EDB_CAM-B_pojezd_kamery_noze.pdf)

**SCHÉMA PCB:**
POZOR! nezapojovat R23 a R29. (LS1-OS a LS2-DS), ty nabíjí piny GPIO12 a GPIO13 a mikrokontroler potom nechce bootovat.
Pro koncové snímače použít vstupy LS2 a LS3.
Dále není ve schématu spojení pinů MS1,MS2 a Vcc u driverů motorů. Ty jsou tak ve full-step režimu.
[Schematic_linear travel_2021-02-25.pdf](https://github.com/panMasinka/ESP32_linear_travel_v2/files/7022646/Schematic_linear.travel_2021-02-25.pdf)

**DRIVER:**

![A4988-Stepper-Driver1](https://user-images.githubusercontent.com/53040547/130250913-9817a3c7-f182-4ff4-b56b-21a685e2d233.jpg)

![A4988-Specsifications](https://user-images.githubusercontent.com/53040547/129195844-558fbcbb-ea0c-4d48-9e05-0b6315cee30a.png)

základní zapojení:
V základním zapojení je driver řízen tak, že pin DIR určuje směr otáčení a pin STEP určuje rychlost. Každá náběžná hrana na pinu STEP znamená 1 krok, nebo 1 mikrokrok (podle zapojení pinů pro microstepping). Jeden celý krok je zmněna natočení hřídele o 1,8°.

![0J10073 600](https://user-images.githubusercontent.com/53040547/130259376-980ae84f-a294-49f1-8cc2-cbf122744a32.jpg)


**A4988 microstepping** . . . . oproti schématu je potřeba propojit piny MS1 a MS2 s ovládacím napětím 3.3V

![A4988-Truth-Table1-300x227](https://user-images.githubusercontent.com/53040547/130251128-a9126449-7a1e-4779-8c4f-b232dab754f4.png)

Na desce pojezd kamer okrajků je propojení pinů MS1, MS2 a Vcc řešeno přímo na driveru
![IMG_20210625_210154](https://user-images.githubusercontent.com/53040547/130251799-0bc363d6-4420-4acf-bf98-9ba6ddc9ac32.jpg)

Na náhradní desce je to řešeno přímo na PCB ze spodní strany.

![image29](https://user-images.githubusercontent.com/53040547/130277707-a6537a36-851a-4732-b6d4-ff662e314af6.png)



**NASTAVENÍ PRACOVNÍHO PROUDU MOTORŮ:**
Proud, který teče do motoru je na driveru A4988 nastaven referenčním napětím měřitelným mezi trimerem a nulou ovládacího napětí.
Pro motor NEMA17 42-40 je to Vref= 0,72V.
Referenční napětí se nastavuje v klidovém stavu, kdy na vstupních svorkách X1-X4 ani na svorce EN není přítomno napětí. Tzn motory nejsou v záběru.
Nižší napětí bude znamenat menší kroutící moment a není nijak na škodu. Vyšší napětí by mohlo poškodit driver nebo motor.

![Vref_meas](https://user-images.githubusercontent.com/53040547/130278094-89ad0a81-b919-45b8-a20c-133b34e1ed79.png)



DESKA PLOŠNÝCH SPOJŮ

![PCB_detail](https://user-images.githubusercontent.com/53040547/130277593-4cf06fa2-3571-47ad-8a5d-1fbb3fd103ac.png)


