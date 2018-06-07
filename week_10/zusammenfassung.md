# Zusammenfassung

Der größte Performance-Gewinn ist auf „Tiling“ zurückzuführen. Die Anzahl der zu verarbeiteten „Tiles“ ergibt sich aus der Größe der Matrix durch die Größe eines „Tiles“, wobei ein „Tile“ so groß wie eine „Local Work Group“ ist.

Im Folgenden wird über die Anzahl der „Tiles“ iteriert und es werden zwei „Local-Memory-Matrizen“ `sub_a`, `sub_b` befüllt. Danach wird über ein einzelnes „Tile“ iteriert und die zwei Matrizen an der jeweiligen Spalte und Zeile werden multipliziert, aufsummiert und der Variable `acc` zugewiesen. Im letzten Schritt wird der Wert von `acc` in der Zielmatrix eingefügt.

Für die „Local Work Group Size“ wird die maximal unterstützte „Local Work Group Size“ des „Devices“ verwendet. Die „Global Work Size“ wird so gewählt, dass sich die Problemgröße der Matrix ganzzahlig durch die „Local Work Group Size“ dividieren lässt.

## Benchmark on Vega 64 / i9 7900x

Setting up n=500 ..<br/>
  CPU setup took 0.002s / 138.884 GFLOPS<br/>
  Duration: 0.000s, GFLOPS: 507.416, Verification: OK<br/>
  Duration: 0.000s, GFLOPS: 500.056, Verification: OK<br/>
  Duration: 0.001s, GFLOPS: 483.316, Verification: OK<br/>
  Performance result for n=500: 507.416<br/>

Setting up n=734 ..<br/>
  CPU setup took 0.008s / 102.527 GFLOPS<br/>
  Duration: 0.001s, GFLOPS: 1216.730, Verification: OK<br/>
  Duration: 0.001s, GFLOPS: 1110.762, Verification: OK<br/>
  Duration: 0.001s, GFLOPS: 1211.214, Verification: OK<br/>
  Performance result for n=734: 1216.730<br/>

Setting up n=1024 ..<br/>
  CPU setup took 0.019s / 111.674 GFLOPS<br/>
  Duration: 0.002s, GFLOPS: 1204.651, Verification: OK<br/>
  Duration: 0.002s, GFLOPS: 1205.652, Verification: OK<br/>
  Duration: 0.002s, GFLOPS: 1198.977, Verification: OK<br/>
  Performance result for n=1024: 1205.652<br/>

Setting up n=1493 ..
  CPU setup took 0.052s / 127.335 GFLOPS<br/>
  Duration: 0.006s, GFLOPS: 1152.434, Verification: OK<br/>
  Duration: 0.006s, GFLOPS: 1118.999, Verification: OK<br/>
  Duration: 0.006s, GFLOPS: 1141.523, Verification: OK<br/>
  Performance result for n=1493: 1152.434<br/>

Setting up n=2345 ..<br/>
  CPU setup took 0.339s / 76.060 GFLOPS<br/>
  Duration: 0.024s, GFLOPS: 1073.503, Verification: OK<br/>
  Duration: 0.024s, GFLOPS: 1075.511, Verification: OK<br/>
  Duration: 0.024s, GFLOPS: 1078.106, Verification: OK<br/>
  Performance result for n=2345: 1078.106<br/>

Setting up n=4001 ..<br/>
  CPU setup took 2.595s / 49.365 GFLOPS<br/>
  Duration: 0.121s, GFLOPS: 1060.467, Verification: OK<br/>
  Duration: 0.129s, GFLOPS: 992.833, Verification: OK<br/>
  Duration: 0.130s, GFLOPS: 985.980, Verification: OK<br/>
  Performance result for n=4001: 1060.467<br/>

-------------------------------------------------
Overall result: 996.764 GFLOPS
