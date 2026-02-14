# RC-CAR
Najlepsze autko na świecie

**Nie pushujcie prosze .vscode na repo, bo wtedy klonujacy/pullujacy bedzie mial wasz config sciezek do kompilatora i bedzie musial go znowu ustawiac pod siebie**<br>
*Zeby zmiany w configu sie nie pokazywaly w source control to wpiszcie to w terminal:*
```
    git update-index --skip-worktree .\.vscode\c_cpp_properties.json
```  
## Pinout (Arduino Nano / ATmega328P)

### IR (NEC)
- **D2 (PD2 / INT0)**: wejście z odbiornika IR (TSOP itp.)

### PWM do mostków H (4x PWM: przód/tył dla lewej i prawej strony)
- **Lewa strona (Timer1)**
  - **D9  (PB1 / OC1A)**: LEFT_FWD (PWM)
  - **D10 (PB2 / OC1B)**: LEFT_REV (PWM)
- **Prawa strona (Timer2)**
  - **D3  (PD3 / OC2B)**: RIGHT_FWD (PWM)
  - **D11 (PB3 / OC2A)**: RIGHT_REV (PWM)

> Ważne: dla jednej strony auta nie ustawiaj naraz FWD i REV > 0 (ryzyko hamowania / duże prądy / zwarcie w niektórych mostkach).

Bardzo ważne: wspólna masa:
GND Arduino ↔ GND mostków ↔ GND zasilania silników

### Szybkie odwrócenie kierunku (gdy przewody są "na odwrót")
W `motor_control.h` masz:
- `MOTOR_LEFT_INVERT`
- `MOTOR_RIGHT_INVERT`

Ustaw na `1` dla tej strony, która kręci się odwrotnie. Bez przepinania kabli.

Kiedy auko ściąga na bok to można to porpawić łatwo na przykład tak: right = right * 0.95

**Krok A — podłącz IR i odpal UART**<br>

IR receiver:

VCC → 5V

GND → GND

OUT → D2

Odpal program, otwórz Serial Monitor / PuTTY na 115200.

Naciskaj przyciski pilota

Będziesz widział linie typu:
```
IR addr=0x00FF cmd=0x18
IR addr=0x00FF cmd=0x52 R
```
cmd=0x18 to kod przycisku
R oznacza repeat (trzymasz przycisk)

**Krok B — wpisz kody do define’ów**<br>

W main.c masz coś w stylu:
```
#define CMD_FORWARD   0x00
#define CMD_BACKWARD  0x00
#define CMD_LEFT      0x00
#define CMD_RIGHT     0x00
#define CMD_STOP      0x00
```
Wpisujesz tam to co zobaczyłeś na UART (same f.command).


**Krok C — (opcjonalnie, ale polecam) filtruj po adresie**<br>

Jeśli chcesz mieć 100% pewności, że nie reagujesz na inne piloty, dodaj warunek:

zapamiętaj addr, który widzisz zawsze (np. 0x00FF)

i reaguj tylko, jeśli f.address == TEN_ADRES


Przeredagowac readme.md
    -wstep czym jest projekt
    -co wykorzystujemy fizycznie(elementy, ardunio nano/uno r3, 2/4 silniczki, jeden mostek H na strone - przod i tyl sterowane tak samo,  korzystamy z NEC'a infrared - definicje komend z pilota sa main.c - proghram wypisuje serial'em odbierane komendy)
    -definicje pinow domyslnych(np. D11 - lewy pwm do przodu)
    -sterowanie(sterujemy pilotem od telewizora, program obsluguje komendy repeat wysylane przez pilot - tak dlugo jak trzymany jest przycisk tak dlugo wykonywana jest komenda)
    -zasilanie z BMS(18650 Li-Ion x 4), silniczki 4-6V, mostek H 6-27V, mostek H zasilony bezposrednio z BMS'a(~16V) - sterowany PWM'em o wypelnieniu 35% w celu uzyskania na silniczkach ~5V
    -kod napisany modularnie, wszystkie wazne wartosci zamykane w definicjach zmiennych na gorze plikow 
