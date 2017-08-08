# vsp_simulate #
用于在 PC 上模拟效果和做性能评估, 做算法定点化, 并使用 NatureDSP 库优化关键路径的性能, 直到性能满足要求。指标一般是对 10ms 数据, 处理时间低于 10ms

## 编译和运行 ##
1. cd tb
2. make
3. xt-run ./output/dsp_tb.elf ./audio_data/

## 生成 profile ##
1. make profile

