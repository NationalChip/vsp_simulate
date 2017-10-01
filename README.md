# vsp_simulate #
用于在 PC 上模拟效果和做性能评估, 做算法定点化, 并使用 NatureDSP 库优化关键路径的性能, 直到性能满足要求。指标一般是对 10ms 数据, 处理时间低于 10ms

## 编译和运行 ##
1. cd tb
2. make
3. xt-run ./output/dsp_tb.elf ./audio_data/

## 生成 profile ##
1. make profile

## 相关参数配置 ##
1. 配置 mic 数跟 ref 数：修改 vsp_main.c 中的宏 MIC_NUM(默认为4) 和 REF_NUM(默认为2)
2. 配置 ctx 和 channel 数：修改 vsp_context.c 中的宏 FRAME_NUM_PER_CONTEXT(默认为3) 和 CONTEXT_NUM_PER_CHANNEL(默认为5)
3. 配置帧长：修改 vsp_context.c 中的宏 FRAME_LENGTH(默认为10)， 单位是 ms
