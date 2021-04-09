if ! command -v zip &> /dev/null
then
	echo "install zip command"
	exit
fi

config_file=~/.config/paradigms

if [[ -f "$config_file" ]]; then
	source ~/.config/paradigms
fi

if [[ -z "$emailid" ]]; then
	read -p "enter id: " emailid
	echo "emailid=$emailid" > ~/.config/paradigms
fi

zip $emailid.zip rsg.cc
