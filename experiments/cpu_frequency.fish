#!/usr/bin/env fish

echo "Core | Freq (GHz) | IRQs |"
echo "-----|------------|------|"

for core in (seq 0 31)
    set temp_file (mktemp)
    sudo perf stat -e cycles:u -e irq:irq_handler_entry -e irq:softirq_entry \
        build/cpptail --core $core 2>$temp_file >/dev/null
    
    # Extract cycles and time to calculate GHz
    set cycles (grep "cycles:u" $temp_file | awk '{print $1}' | tr -d ',')
    set time (grep "seconds time elapsed" $temp_file | awk '{print $1}')
    set ghz (math "$cycles / $time / 1000000000")
    
    # Extract hardware and software IRQ counts
    set hw_irq (grep "irq:irq_handler_entry" $temp_file | awk '{print $1}' | tr -d ',')
    set sw_irq (grep "irq:softirq_entry" $temp_file | awk '{print $1}' | tr -d ',')
    
    # Sum IRQs (handle empty values)
    set hw_irq (test -n "$hw_irq"; and echo $hw_irq; or echo 0)
    set sw_irq (test -n "$sw_irq"; and echo $sw_irq; or echo 0)
    set total_irq (math "$hw_irq + $sw_irq")
    
    rm $temp_file
    
    printf "%4d | %10.3f | %4d |\n" $core $ghz $total_irq
end
