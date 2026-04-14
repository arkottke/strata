# Disable downloading debug info for system libraries
set debuginfod enabled off

# Skip standard library and third-party headers
skip -gfi /usr/include/c++/*
skip -gfi /usr/include/x86_64-linux-gnu/*
skip -gfi /usr/lib/*
skip -gfi /usr/*
skip -gfi /opt/*
