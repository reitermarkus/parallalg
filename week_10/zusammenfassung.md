# Zusammenfassung

Der größte Performance-Gewinn ist auf „Tiling“ zurückzuführen. Die Anzahl der zu verarbeiteten „Tiles“ ergibt sich aus der Größe der Matrix durch die Größe eines „Tiles“, wobei ein „Tile“ so groß wie eine „Local Work Group“ ist.

Im Folgenden wird über die Anzahl der „Tiles“ iteriert und es werden zwei „Local-Memory-Matrizen“ `sub_a`, `sub_b` befüllt. Danach wird über ein einzelnes „Tile“ iteriert und die zwei Matrizen an der jeweiligen Spalte und Zeile werden multipliziert, aufsummiert und der Variable `acc` zugewiesen. Im letzten Schritt wird der Wert von `acc` in der Zielmatrix eingefügt.

Für die „Local Work Group Size“ wird die maximal unterstützte „Local Work Group Size“ des „Devices“ verwendet. Die „Global Work Size“ wird so gewählt, dass sich die Problemgröße der Matrix ganzzahlig durch die „Local Work Group Size“ dividieren lässt.
