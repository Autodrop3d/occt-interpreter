
BASEDIR=$(dirname "$0")
ROOT=$BASEDIR"/.." 
PCKG=$ROOT"/npm-package" 
GIT_SHA=`git rev-parse HEAD`

mkdir $PCKG"/package"
cp -r $PCKG"/template/." $PCKG"/package"
cp $ROOT"/build-wasm/main.js" $PCKG"/package/occt.js"
cp $ROOT"/build-wasm/main.wasm" $PCKG"/package/occt.wasm"



sed -i -e "s/<SNAPSHOT_SHA>/"$GIT_SHA"/g" $PCKG"/package/package.json"

ls $PCKG"/package/"

cd $PCKG"/package/"
npm publish --access public
cd -
git push
