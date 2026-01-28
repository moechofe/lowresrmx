
DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

IN="${ROOT}/asset.dev/manual.md"
# IN="${DIR}/test.md"
OUT="${ROOT}/assets.dev/manual_out.md"

# get keywords
cat "$IN"\
|grep -A6 "Reserved keywords"\
|grep 'PALETTE'\
|sed 's/(#*[a-z0-9-]*)//g; s/[\[`]//g; s/\]//g; s/.$//'\
|sed 's/[,\&]/\n/g'\
|sed 's/\$//; s/=//'\
|grep -v 'TODO'\
|sed 's/ *//'\
|sort\
>/tmp/manual1

# get header
cat "$IN"\
|grep '^#### `'\
>/tmp/manual2

echo "">/tmp/manual4
echo "">/tmp/manual5

# fix header and generate index
cat /tmp/manual2\/
|while read line; do

	# echo
	# echo "=> $line"

	raw="$(echo "$line"\
	|sed 's/^#### *//')"
	# echo "$raw"

	echo "$raw"\
	|sed 's/<br>/\n/g'\
	>/tmp/manual3
	header="$(cat /tmp/manual3\
	|sed 's/^`//g; s/`$//g'\
  |sed 's/\([a-z0-9]\),\([a-z0-9]\)/\1, \2/g; s/\([a-z0-9]\)\[,/\1 [,/g; s/,\[/, [/g'\
	|sed 's/\$,/$, /g'\
	|sed 's/[^ ]=/ =/g; s/\]\([a-z0-9]\)/] \1/g'\
	|sed 's/^`//g; s/`$//g'\
	|sed 's/^\(.*\)$/`\1`/'\
	|paste -s -d '|' -\
	|sed 's/|/<br>/g')"
	# echo $header

	# |sed 's/^`/ /; s/`$/ /'\
	link="$(echo "$header"\
	|sed 's/[^a-z0-9A-Z ]/-/g'\
	|tr '[:upper:]' '[:lower:]'\
	|sed 's/  */-/g'\
	|sed 's/---*/-/g')"
	# echo link:$link

	# fixed header

	echo -n "$line">>/tmp/manual4
	echo -ne "\t">>/tmp/manual4
	echo "#### $header">>/tmp/manual4

	# generated index

	if [[ "$header" =~ \` ]]; then

		# echo -n "$header"\
		# |sed 's/[^A-Z]*//g'>>/tmp/manual5
		# echo -en "\t">>/tmp/manual5
		echo "[$header](#$link)">>/tmp/manual5
	fi

done

# repair header

cat "$IN"\
|while IFS='' read -r line; do
	found=
	while IFS=$'\t' read search replacement; do
		if [ "$line" = "$search" ]; then
			echo "$replacement"
			found="ok"
			break
		fi
	done < /tmp/manual4
	if [[ -z "$found" ]]; then
		echo -n "$line"
		echo ""
	fi
done

# add index

cat /tmp/manual1\
|while read token; do

	echo "**\`$token\`**:"
	grep "\b$token\b" /tmp/manual5\
	|sed 's/^/- /'
	[[ ${PIPESTATUS[0]} -eq 0 ]] || echo "- [\`$token\`](#operators)"

done
