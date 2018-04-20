# Benchmark on GeForce GTX 1070

## Local vs Global Memory benchmark

### Local Memory

| Problem Size | Work Group Size | Time          | MFLOPS     |
|--------------|-----------------|---------------|------------|
|          264 | 4               |    674.412 ms |  19220.793 |
|          500 | 4               |   2575.585 ms |  33972.860 |
|         1000 | 4               |  21988.531 ms |  31834.778 |

### Global Memory

| Problem Size | Work Group Size | Time          | MFLOPS     |
|--------------|-----------------|---------------|------------|
|          264 | 4               |    591.592 ms |  21771.474 |
|          500 | 4               |   1908.047 ms |  45858.404 |
|         1000 | 4               |  16244.587 ms |  43091.276 |

## Work Group Size benchmark

| Problem Size | Work Group Size | Time          | MFLOPS     |
|--------------|-----------------|---------------|------------|
|          500 | 2               |   7983.239 ms |  10960.460 |
|          500 | 4               |   2569.122 ms |  34058.330 |
|          500 | 10              |   1745.661 ms |  50124.293 |
