# ./plot/access_patterns.py --path=/root/memsim/bench/nskipped_patterns/takes-100-th/ --region-start=140087386791936 --region-size=1073741824
# ./plot/access_patterns.py --path=/root/memsim/bench/nskipped_patterns/takes-10-th/ --region-start=140657550966784 --region-size=1073741824
./plot/access_patterns.py --path=/root/memsim/bench/nskipped_patterns/takes-4-th/ --region-start=140251806744576 --region-size=1073741824
# ./plot/access_patterns.py --path=/root/memsim/bench/nskipped_patterns/takes-2-th/ --region-start=140485631807488 --region-size=1073741824
# ./plot/access_patterns.py --path=/root/memsim/bench/nskipped_patterns/skips-4-th/ --region-start=140038850437120 --region-size=1073741824
./plot/access_patterns.py --path=/root/memsim/bench/nskipped_patterns/skips-1-th/ --region-start=140053417287680 --region-size=1073741824


# [root@localhost bench]# ./run/nskipped_patterns.py
# /root/memsim/bench
# ['skips-4-th', 'skips-10-th', 'skips-100-th', 'skips-1-th', 'takes-100-th', 'takes-10-th', 'takes-4-th', 'takes-2-th']

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -skip-every-nth 1 -o /root/memsim/bench/nskipped_patterns/skips-1-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 140053417287680 or 0x7f60ba2fe000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 38856097504\nWriting observed values.\nTime taken to write 148399003602\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -skip-every-nth 4 -o /root/memsim/bench/nskipped_patterns/skips-4-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 140038850437120 or 0x7f5d55ef6000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 31571982364\nWriting observed values.\nTime taken to write 111627690656\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -take-every-nth 2 -o /root/memsim/bench/nskipped_patterns/takes-2-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 140485631807488 or 0x7fc55c2e1000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 22189782338\nWriting observed values.\nTime taken to write 84674975362\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -take-every-nth 4 -o /root/memsim/bench/nskipped_patterns/takes-4-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 140251806744576 or 0x7f8eeb1ec000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 13339753062\nWriting observed values.\nTime taken to write 37929007972\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -take-every-nth 10 -o /root/memsim/bench/nskipped_patterns/takes-10-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 140657550966784 or 0x7fed635c4000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 8196439614\nWriting observed values.\nTime taken to write 15135630728\n'
    
# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -take-every-nth 100 -o /root/memsim/bench/nskipped_patterns/takes-100-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 140087386791936 or 0x7f68a2ed6000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 4062985550\nWriting observed values.\nTime taken to write 1537018098\n'







# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -skip-every-nth 10 -o /root/memsim/bench/nskipped_patterns/skips-10-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 139686436843520 or 0x7f0b4872b000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 36977920396\nWriting observed values.\nTime taken to write 133319525604\n'

# Running experiment:
# pin -t /root/memsim/src/obj-intel64/main.so -skip-every-nth 100 -o /root/memsim/bench/nskipped_patterns/skips-100-th -- /root/memsim/apps/gups/obj/gups_rndm_write.o 1 1000000 30 45 10 50
# Try # 1
# b'Region start: 139915901186048 or 0x7f40b596b000 \tRegion size: 1073741824\nGUPS finished safely.\nEnd-to-End exection time - 37279658596\nWriting observed values.\nTime taken to write 147576108094\n'
