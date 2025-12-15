#!/usr/bin/env fish

echo "Core | Mean (ns) | p50 (ns) | p99 (ns) | p99.9 (ns)"
echo "-----|-----------|----------|----------|------------"

for core in (seq 0 (math (nproc) - 1))
    build/cpptail --core $core | while read -l line
        if string match -q "*Mean time:*" $line
            set mean (echo $line | awk '{print $3}')
        else if string match -q "*p50 time:*" $line
            set p50 (echo $line | awk '{print $3}')
        else if string match -q "*p99 time:*" $line  
            set p99 (echo $line | awk '{print $3}')
        else if string match -q "*p99.9 time:*" $line
            set p999 (echo $line | awk '{print $3}')
        end
    end
    
    printf "%4d | %9s | %8s | %8s | %10s\n" $core $mean $p50 $p99 $p999
end
