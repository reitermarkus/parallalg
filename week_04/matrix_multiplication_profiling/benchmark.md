# Benchmark on Vega 64

| Problem Size | Kernel Time   | Write Time   | Write Speed | Read Time   | Read Speed | GPU Time      | GPU + CPU Time | MFLOPS     |
|--------------|---------------|--------------|-------------|-------------|------------|---------------|----------------|------------|
|          100 |   0.076516 ms |  0.020720 ms |   3,86 GB/s | 0.006216 ms |  6,43 GB/s |   0.103452 ms |      13.207 ms |    151.435 |
|          500 |   0.566840 ms |  0.155400 ms |  12,87 GB/s | 0.090132 ms | 11,09 GB/s |   0.813556 ms |      13.641 ms |  18326.942 |
|         1000 |   6.118172 ms |  0.598216 ms |  13,37 GB/s | 0.284456 ms | 14,06 GB/s |   7.000844 ms |      22.078 ms |  90587.763 |
|         2500 |  70.731124 ms |  3.708140 ms |  13,48 GB/s | 1.746548 ms | 14,31 GB/s |  76.185812 ms |     105.803 ms | 295360.209 |
|         5000 | 466.442128 ms | 14.818352 ms |  13,49 GB/s | 6.984120 ms | 14,31 GB/s | 488.244600 ms |     581.805 ms | 429697.242 |
