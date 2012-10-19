#!/bin/sh
# This sets up the post-commit hooks to send mail and notify KGB,
# for all d-i repos. It is designed to be run again when adding a new repo.

set -e

cd /git/d-i

SAVE_UMASK=`umask`
umask 007

cat > kgb-client.conf.new <<EOF
# Generated file. See {svn}/scripts/hooksetup.sh
#
repo-id: debian-boot
timeout: 7
servers:
# dam, KGB-0
 - uri: http://nose.ktnx.net:9418/
# gregoa, KGB-2
 - uri: http://colleen.colgarra.priv.at:8080/
# Tincho, KGB-1
 - uri: http://abhean.mine.nu:9418/
web-link: http://git.debian.org/?p=d-i/\${module}.git;a=commitdiff;h=\${commit}
short-url-service: Debli
EOF

echo "cnffjbeq: Vrq0nvxr" | tr A-Za-z N-ZA-Mn-za-m >> kgb-client.conf.new
mv -f kgb-client.conf.new kgb-client.conf

umask $SAVE_UMASK

# Where the kgb-bot lives, until it's installed system-wide.
KGB=/home/groups/kgb/trunk

for repo in *.git attic/*.git; do
	cd /git/d-i/$repo

	proj=$(echo $repo | sed 's/\.git$//')

	# TODO search for [l10n] and SILENT_COMMIT in commit message; send
	# former to bubulle and drop latter.
	rm -f hooks/post-receive.new
	cat > hooks/post-receive.new <<EOF
#!/bin/sh
# Do not edit this file; it is regenerated by scripts/hooksetup.sh
EOF
	echo "PERL5LIB=$KGB/lib $KGB/script/kgb-client --conf /git/d-i/kgb-client.conf --git-reflog -" >> hooks/post-receive.new
	echo "exec /usr/local/bin/git-commit-notice" >> hooks/post-receive.new
	chmod 755 hooks/post-receive.new
	mv -f hooks/post-receive.new hooks/post-receive

	cat > hooks/post-update.new <<EOF
#!/bin/sh
# Do not edit this file; it is regenerated by scripts/hooksetup.sh
exec git update-server-info
EOF
	chmod 755 hooks/post-update.new
	mv -f hooks/post-update.new hooks/post-update

	./hooks/post-update

	git config --replace-all hooks.mailinglist "debian-installer_cvs@packages.qa.debian.org"
	git config --replace-all hooks.replyto "debian-boot@lists.debian.org"
	# remove obsolete CIA hook
	git config --unset hooks.cia-project || [ $? = 5 ]  # do not die when not set

	echo "d-i $proj repository" > description
done
