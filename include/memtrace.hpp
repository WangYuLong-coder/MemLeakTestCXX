#pragma once

extern "C" {

void *__wrap_malloc(size_t size);
void *__wrap_calloc(size_t num, size_t size);
void *__wrap_realloc(void* ptr, size_t new_size);
void __wrap_free( void * ptr );

void report_leaks();

} // extern "C"

