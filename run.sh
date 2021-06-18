self=$(basename $0)
folder=$1

cd $folder
make

for file in $(ls);do
    if [ -x "$file" ] && [ "$file" != "$self" ]; then
        echo "\nRun $file:"
        ./$file
        echo "\n"
    fi
done

make clean
cd ..