## Step 1) Finden der höchsten Zahl

```c
  for (int i = 0; i < size; i++) {
    if (input[i].age > max)
      max = input[i].age;
  }
```
Kann mit Synchronisierung der Variable "max" parallelisiert werden.


## Step 2) Häufigkeit abzählen

```c
  for (int i = 0; i < size; i++) {
    count_arr[input[i].age]++;
  }
```
Kann parallelisiert werden, da die workitems unabhängig voneinander arbeiten.


## Step 3) Berechne für jedes Element e in C, die Anzahl von Einträgen, die im Array A und kleiner dem Element e sind

```c
  for (int i = 0; i <= max; i++) {
    int count = 0;
    for (int j = 0; j < size; j++) {
      if (input[j].age < i)
        count++;
      }
      count_arr[i] = count;
  }
```
Kann mit Synchronisierung der Variable "count" und barrier(..) nach der inneren Schleife parallelisiert werden.


## Step 4) Elemente in richtiger Reihenfolge im Ergebnisarray einsetzen

```c
  for (int i = 0; i < size; i++) {
    person_t p = input[i];
    result[count_arr[p.age]++] = p;
  }
```
Kann mit Synchronisierung des resul- und count_arr-Arrays parallelisiert werden, aber dies könnte ineffizient sein.
