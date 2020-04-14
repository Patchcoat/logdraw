#ifndef MERGESORT_H
#define MERGESORT_H
// ****************************************************************************
// Merge Sort the format function arrays
// This sorts one array using another array
// ****************************************************************************
// sorts the working array into the output array
void mrg(char** work, unsigned short* work_,
         size_t start, size_t mid, size_t end,
         char** out, unsigned short* out_) {
    size_t i = start;
    size_t j = mid;

    for (size_t k = start; k < end; k++) {
        // if the left run head exists and is <= the existing right run head
        // NULL is weighted to be at the end of the list
        // If the right run is null or the left run is not null then the left
        // run must be greater than the right run or the right run is null. If
        // that is true then use the left run, otherwise use the right run.
        if (i < mid && (j >= end ||
                    ((work[j] == NULL || work[i] != NULL) &&
                     (work[i] <= work[j] || work[j] == NULL)))) {
            out_[k] = work_[i];
            out[k] = work[i++];
        } else {
            out_[k] = work_[j];
            out[k] = work[j++];
        }
    }
}

// sort out array using the working array
// start in inclusive, end is exclusive
void spltmrg(char** work, unsigned short* work_,
             size_t start, size_t end,
             char** out, unsigned short* out_) {
    if (end - start < 2) // if the size is 1 it's sorted
        return;
    // split it in half
    size_t mid = (end + start) / 2;
    // recursively sort the halves
    spltmrg(out, out_, start, mid, work, work_);
    spltmrg(out, out_, mid, end, work, work_);

    // merge the two runs
    mrg(work, work_, start, mid, end, out, out_);
}

// this takes in the two arrays, the array to sort by (in) and the array that
// is being sorted by, (in_). It then creates working arrays and copies over
// the input arrays into those. It then calls a function to sort the working
// arrays into the input arrays
void mrgsrt(char** in, unsigned short* in_, size_t n) {
    char* work[n];
    unsigned short work_[n];
    memcpy(work, in, sizeof(char*)*n);
    memcpy(work_, in_, sizeof(unsigned short)*n);
    spltmrg(work, work_, 0, n, in, in_);
}
// ****************************************************************************
// End Merge Sort
// ****************************************************************************
#endif
