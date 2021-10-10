all: get_query len_query set_query

execute_query.o: execute_query.c execute_query.h
	gcc -c $<

len_query: execute_query.o len_query.c
	gcc $^ -o $@

get_query: execute_query.o get_query.c 
	gcc $^ -o $@

set_query: execute_query.o set_query.c
	gcc $^ -o $@

clean:
	rm -f execute_query.o len_query get_query set_query

.PHONY: clean