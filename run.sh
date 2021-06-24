run_dir() {
    local folder=$1
    cd $folder
    make

    for file in $(ls); do
        if [ -x "$file" ] && [ "$file" != "$self" ]; then
            echo "\nRun $file:"
            ./$file
            echo "\n"
        fi
    done

    make clean
    cd ..
}

self=$(basename $0)

if [ -z "$1" ] ; then
    echo "Please set a directory"
elif [ ! -d "$1" ]; then
    echo "$1 is not a directory"
else
    echo "Run examples in $1:\n==========\n"
    run_dir $1
fi
