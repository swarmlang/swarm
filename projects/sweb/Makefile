
SWARMC := ../../swarmc_debug

sweb: sweb.swarm
	$(SWARMC) --binary sweb sweb.swarm

dump_svi: sweb.swarm
	$(SWARMC) --dbg-output-isa-to sweb.svi sweb.swarm
	node ../../bin/prettify_isa.js sweb.svi > sweb.svi.2
	mv sweb.svi.2 sweb.svi

run_redis: sweb
	$(SWARMC) --redis sweb

run_locally: sweb
	$(SWARMC) --locally sweb
