# Ring Buffer

[`SPSCRingBuffer`][ring_buffer] is a single-producer-single-consumer(*SPSC*) circular queue. The data is produced and consumed in first-in-first-out (*FIFO*) order.

## Fast-producer-slow-consumer Problem

The ideal scenario is that the data producing rate is equal to the data consuming rate. As a result, the data generated can be processed in real-time.

However, if the data-producer produces data faster than what data-consumer can consume, then the unprocessed data in the ring buffer will keep growing and get full at the end. Consequently, the data-producer is unable to write any new data into the ring-buffer.

### Data-producing rate control

#### Batching data

Although the ring buffer size is a constant value defined in its declaration, we still have a way to grow its size: by batching data dynamically.

Suppose the data entry of the ring buffer is a list or an array whose size is resizable. In that case, the data stored in the ring buffer is also resizable because one data entry can contain multiple data. In other words, if the data entry of the L-sized ring buffer has N data, then the data we can store in the ring buffer is L x N.

If the data-producer produces M data per second, then the data-producing rate for the data entry is M / N since every data entry has N data. The data-producer delivers 1 / M entry every round instead of 1 entry every round. This means we batch M data into one entry of the ring buffer.

#### Batch resizing

But how do we know it's time to increase or decrease the batch size? The simplest way is to set a threshold T, a size smaller than the buffer size. We can monitor the size of the unconsumed entries. If the size is over T, then we guess the data producing rate is higher than the data consuming rate. That means we need to slow down the production rate until it is equal to or lower than the consumption rate.

If the size of the unconsumed entries is always smaller than the threshold T we used to have, we guess the data producing rate is slower than the data consuming rate. It's better to speed up the data producing rate to process the data generated in real-time.

### Assumption

The producing-rate and consumeing-rate are unknown. (so producing-rate/consumeing-rate is unknown)

### Goal

- Consume the produced data as quick as possible
- Read and write always work (infallible) so consumer and producer won't be blocked
- Do not drop data

### Producing-rate/Consumeing-rate changes randomly

[`DynamicRingBuffer`][dyn_ring_buffer] could be used when *producing-rate / consumeing-rate* changes frequently. If the `capacity` is set to *2^k*, then
- the `batch_size` / `batch_size` changes *k+1* times
- There are *k* section with `capacity / 2` (*2^(k-1)*) items and one (last) section with `capacity` (*2^k*) samples
- Number of total items *T* is _k * 2^(k-1) +  2^k = (k/2 + 1) * 2^k_
- If *producing-rate / consumeing-rate < T*, then the ring-buffer should be read at least once before the ring-buffer is full
- If *producing-rate* in first section is *r*, then the *producing-rate* in *n* section is *producing-rate / 2^n*

### Producing-rate/Consumeing-rate is constant

When the *producing-rate/consumeing-rate* is roughly fixed, use [`DynamicRingBuffer`][dyn_ring_buffer] to estimate the *producing-rate / consumeing-rate = R* first and then set `batch_size` to *ceil(R / ring-buffer capacity)*.

Another approach is to estimate *producing-rate* and *consumeing-rate* then calculate the `batch_size` directly.

TODO

- Estimate *producing-rate* and *consumeing-rate* `chrono::high_resolution_clock`
- Then set `batch_size` to *ceil(R / ring-buffer capacity)*, where *R = producing-rate / consumeing-rate*

### Performance

TODO

- Algorithm Analysis
- Compare between different approaches (at least we cna have two approaches when *producing-rate/consumeing-rate* is roughly fixed)

[ring_buffer]: ring_buffer.h
[dyn_ring_buffer]: dynamic_ring_buffer.h
