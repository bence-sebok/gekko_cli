# gekko_cli
A repository a [Budapesti Műszaki és Gazdaságtudományi Egyetem](http://www.bme.hu) [Villamosmérnöki és Informatikai Karán](https://www.vik.bme.hu) a [Méréstechnika és Információs Rendszerek Tanszék](https://www.mit.bme.hu) [Beágyazott és ambiens rendszerek](http://www.mit.bme.hu/oktatas/targyak/vimiac06) tantárgyához készült.
A házi feladat: soros porti parancssor értelmező (command line interpreter) és periféria kezelő egy EFM32GG-STK3700 board-ra.

## Feladatkiírás
Készítsetek egy programot az STK3700 kártyára, amely egy soros porti parancssor értelmezőt (Command Line Interpreter, CLI) valósít meg. A parancssor utasítások fogadására képes. Az utasítások bevitele mindig a >> kezdődő prompt-tal indulhat, melyet az STK3700-as kártya küld ki. A soros porton keresztül bevitt karaktereket (PC-n terminál programban begépelt karakterek) a kártya küldje vissza (echo). A soros port beállítása legyen: 115.2 kbaud, 8N1.
### Lehetséges parancsok
  * Help: segítségével lekérdezhető a létező parancsok listája
  * Set LED x y: az egyik LED értékének beállítása paraméterek: x: 0 vagy 1-es LED, y: állapot (0,1)
  * Get LED x: visszakérdezi a megadott LED állapotát
  * Write Text: az ENTER-ig begépelt karaktersorozatot megjeleníti az LCD kijelző alsó (alfanumerikus) sorában. Ha a karakterlánc hosszabb, mint ami a kijelzőre kifér, oldjátok meg annak úsztatását, 1 másodperces léptetéssel.
  * Egyéb karaktersorozat: nem értelmezett parancs, hatására adjon hibaüzenetet ("invalid command")

## Dokumentáció
A dokumentáció a _docs_ mappában található, [ezen a linken](https://github.com/sbence/gekko_cli/blob/master/docs/gekko_cli%20dokument%C3%A1ci%C3%B3.pdf).
