SUBDIRS = mis prk sssp

SW_DIR=$(PWD)/apps

all:
	for dir in $(SUBDIRS); do \
                $(MAKE) -C $(SW_DIR)/$$dir; \
        done
run:
	qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_base.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_swi.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_lu2.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_base.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_swi.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_lu2.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_base.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_swi.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_lu2.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_lu4.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_lu8.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_cu2.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_sssp_cu4.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_lu4.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_lu8.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_cu2.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_prk_cu4.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_lu4.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_lu8.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_cu2.sh && qsub  -l walltime=22:00:00 -l nodes=s001-n088:ppn=2 -d . run_mis_cu4.sh && qstat -a
clean:
	for dir in $(SUBDIRS); do \
                $(MAKE) clean -C $(SW_DIR)/$$dir; \
        done

