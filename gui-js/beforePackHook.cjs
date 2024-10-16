let glob=require('glob');
exports.default = async function(context) {
  let p=context.packager;
  // if libravel is present in dynamic_libraries, this is a ravel build.
  let files=glob.globSync(p.info.projectDir+"/dynamic_libraries/libravel.*")
  if (files.length>0) {
    console.log('updating appInfo');
    // this doesn't actually work! file is renamed in mkWindowsDist.sh
    p.appInfo.artifactName="ravel-with-plugin-${version}.${ext}";
  }
}
