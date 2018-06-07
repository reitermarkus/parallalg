# Zusammenfassung

Der größte Performance-Gewinn, ist auf "Tiling" zurückzuführen. Die Anzahl der zu verarbeiteten "tiles" ergibt sich aus der Problemgröße des Arrays durch die Größe eines "tiles", wobei ein "tile" so groß wie eine "local work group" ist.

Im folgenden wird über die Anzahl der "tiles" iteriert und es werden zwei "local-memory-Matrizen" "sub_a", "sub_b" befüllt. Im nächsten Schritt wird über ein einzelnes "tile" iteriert und die zwei Matrizen an der jeweiligen Spalte und Zeile werden multipliziert, aufsummiert und der Variable "acc" zugewiesen. Im letzten Schritt wird der Wert von "acc" in der Zielmatrix eingefügt.

Für die "local work group size" wird die maximal unterstützte "local work group size" des "devices" verwendet. Die "global work size" wird so gewählt, dass sich die Problemgröße der Matrix ganzzahlig durch die "local work group size" dividieren lässt.
