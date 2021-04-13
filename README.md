# Bi-Term Topic Model

This c++ project provides a multithreaded BTM CLI application. See the python file for an example for how to use it.

### References

##### [A Biterm Topic Model for Short Texts](https://doi.org/10.1145/2488388.2488514)

> Specifically, in BTM we learn the topics by directly modeling the generation of word co-occurrence patterns (i.e. biterms) in the whole corpus. The major advantages of BTM are that 1) BTM explicitly models the word co-occurrence patterns to enhance the topic learning; and 2) BTM uses the aggregated patterns in the whole corpus for learning topics to solve the problem of sparse word co-occurrence patterns at document-level.

##### [An architecture for parallel topic models](https://doi.org/10.14778/1920841.1920931)

> The key idea for parallelizing the sampler in the multicore setting is that the global topic distribution and the topic-word table change only  little given the changes in a single document. Hence, we can assume that n(t) and n(t,w) are essentially constant while sampling topics for a document.
