# ./plot/access_patterns.py --path=/root/memsim/bench/access_patterns/rndm --region-start=139811980419072 --region-size=1073741824
# ./plot/access_patterns.py --path=/root/memsim/bench/access_patterns/seq --region-start=140172398575616 --region-size=1073741824
./plot/access_patterns.py --path=/root/memsim/bench/access_patterns/hotset --region-start=140428664774656 --region-size=1073741824



# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -o /root/memsim/bench/access_patterns/rndm -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 0 0 0
# Try # 1
# b'Starting GUPS\nRegion start: 139811980419072 or 0x7f28836da000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 38258739698\nWriting observed values.\nTime taken to write 151333226750\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -o /root/memsim/bench/access_patterns/seq -- /root/memsim/apps/gups/obj/gups_seq_write.o 1 1000000 30 0 0 0
# Try # 1
# b'Starting GUPS\nRegion start: 140172398575616 or 0x7f7c6e064000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 4620913996\nWriting observed values.\nTime taken to write 16254443540\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -o /root/memsim/bench/access_patterns/hotset -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Starting GUPS\nRegion start: 140428664774656 or 0x7fb818ae2000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 40832741458\nWriting observed values.\nTime taken to write 149979776602\n'