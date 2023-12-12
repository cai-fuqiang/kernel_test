Hi baoquan,

In some configurations, out of bounds may not cause crash_exclude_mem_range()
returns error, then the load will succeed.

E.g.
There is a cmem before execute crash_exclude_mem_range():

  cmem = {
    max_nr_ranges = 3
    nr_ranges = 2
    ranges = {
       {start = 1,      end = 1000}
       {start = 1001,	end = 2000}
    }
  }

After executing twice crash_exclude_mem_range() with the start/end params
100/200, 300/400 respectively, the cmem will be:

  cmem = {
    max_nr_ranges = 3
    nr_ranges = 4                    <== nr_ranges > max_nr_ranges
    ranges = {
      {start = 1,       end = 99  }
      {start = 201,     end = 299 }
      {start = 401,     end = 1000}
      {start = 1001,	end = 2000}  <== OUT OF BOUNDS
    }
  }

When an out of bounds occurs during the second execution, the function will not
return error.

Additionally, when the function returns error, means the load failed. It seems
meaningless to keep the original data unchanged. But in my opinion, this will
make this function more rigorous and more versatile. (However, I am not sure if
it is self-defeating and I hope to receive more suggestions).

Thanks
fuqiang
