# RC-CAR
Zdalnie sterowane autko

**To repozytorium ma stanowić prostą bazę do zbudowania własnego, zdalnie sterowanego autka RC.**

## Baza
Bazą projektu jest `Arduino Nano` (lub `uno`, `r3`) / `ATmega328p`.

## Silniki
Silniki autka są sterowane Mostkami H.
- Do kontroli 2 mostków **H** (1 mostek na stronę) Wykorzystuje się `Timer 1` i `Timer 2` 
- **Lewa strona** `Timer1`
  - **D9  (PB1 / OC1A)**: LEFT_FWD (PWM)
  - **D10 (PB2 / OC1B)**: LEFT_REV (PWM)
- **Prawa strona** `Timer2`
  - **D3  (PD3 / OC2B)**: RIGHT_FWD (PWM)
  - **D11 (PB3 / OC2A)**: RIGHT_REV (PWM)

Każdy mostek działa na PWM i kontroluje swój zestaw silników.
- Kontrola przedniego i tylnego rzędu kół jest taka sama w obu mostkach. Kontrola lewej i prawej kolumny kół jest niezależna między nimi.
> Ważne: dla jednej strony auta nie ustawiaj naraz `FWD` i `REV` > 0 (ryzyko hamowania / duże prądy / zwarcie w niektórych mostkach).

### BARDZO WAŻNE
musi być wspólna masa dla:
**GND Arduino** ↔ **GND mostków** ↔ **GND zasilania silników**

## Zasilanie
Autko ma w zamyśle być zasilane z BMS ( 18650 Li-Ion x 4 )

Silniki 4 - 6 V

Mostek H 6 - 27 V
- Mostek H jest bezpośrednio zasilony z BMS'a ( ~16V )
-- Sterowany jest PWM o wypełnieniu `35%` by z 16V otrzymać 5V RMS, które zasila silniki.

## Sterowanie
Autko jest zdalnie sterowane pilotem do telewizora

### Autko skręca na odwrót?
Szybkie rozwiązanie na to jest w pliku `motor_control.h`

Należy ustawić wartość `1` zmiennym:\n
`MOTOR_LEFT_INVERT`
`MOTOR_RIGHT_INVERT`

### Autko ściąga w bok?
Jeżeli autko ściąga na bok, to można ustawić:\n
```right = right * 0.98```\n
To zmniejszy siłę skrętu np. w prawo

## IR
**Sygnał IR od pilota jest obsługiwany protokołem** `NEC infrared`

Odbiornik IR należy podłączyć w poniższy sposób:
```
VCC → 5V
GND → GND
OUT → D2
```

- Definicje komend pilota znajdują się w `main.c`. Należy je dopasować do swojego modelu pilota
- Program wypisuje szeregowo odebrane komendy

### Co jak nie zna się własnych komend pilota?
Wtedy należy:
1. Włączyć UART
2. Włączyć program
3. Otworzyć `Serial Monitor / PuTTY` na porcie `115200`
4. Naciskać przyciski pilota

Otrzymane linijki będą wyglądać w taki sposób:
```
IR addr=0x00FF cmd=0x18
IR addr=0x00FF cmd=0x52 R
```
- `cmd = 0x18` to kod przycisku
- `R` oznacza powtarzanie komendy (przytrzymanie przycisku), program to obsługuje i można to zignorować.

`main.c` zawiera definicje `CMD_` tego typu:
```
#define CMD_FORWARD   0x00
#define CMD_BACKWARD  0x00
#define CMD_LEFT      0x00
#define CMD_RIGHT     0x00
#define CMD_STOP      0x00
```
W `definicję` należy wpisać wynik `cmd=` otrzymany wcześniej.

## Extra
Kod jest napisany modularnie i wszystkie ważne wartości są zamknięte w definicjach zmiennych na górze plików. 


>Informacja dla Contributor'ów
>**Nie pushujcie prosze .vscode na repo, bo wtedy klonujacy/pullujacy bedzie mial wasz config sciezek do kompilatora i bedzie musial go znowu ustawiac pod siebie**<br>
>*Zeby zmiany w configu sie nie pokazywaly w source control to wpiszcie to w terminal:*
```
    git update-index --skip-worktree .\.vscode\c_cpp_properties.json
```

dokumentacja technicza:
- schemat polaczen z kicada(discord UMG)
- opis zasilania plytki i mostkow(mostki z BMS'a, plytka prawdopodobnie z LM7805 na output 5V[żydyzm], enable na mostkach 5V z plytki/LM7805, IR NEC reciever 5V pullup(na schemacie))
- polaczenie mas(BMS z arduino) przez 220ohm resistor
- rozpis elementow - uproszczony datasheet(VS1838 - IR NEC, HW-039 IBT_2 - M mostek H, ardunio nano/uno r3, BMS na 4 akumulatory Li-Ion 18650, 2x deceleration brushed DC motor 4-6V)
- jeszcze raz wspomniec o wypelnieniu pwm'a - silniczki nie wytrzymaja napiecia zasilajacego mostek H, wiec wypelnienie sygnalu sterujecego jest odpowiednio mniejsze zeby uzyskac na silniczkach ~5V)
- bedzie git :D
