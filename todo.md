1 - realloc for arena                                                                           ==> finished
2 - add nonnull to methods                                                                      ==> finished 
3 - benchmark and then decide whether to add branch prediction with '__builtin_expect'          
4 - debug an error which occurs after                                                           ==> finished
    * initializing a vector with vec_ar with 9 elements
    * pushing into the vector
    => error with realloc() : invalid old size
    hint: its prolly related to 1 - realloc for arena
