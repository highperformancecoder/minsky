let glob=require('glob');
exports.default = async function(context) {
  let p=context.packager;
  // if libravel is present in dynamic_libraries, this is a ravel build.
  let files=glob.globSync(p.info.projectDir+"/dynamic_libraries/libravel.*")
  if (files.length>0)
  {
    console.log('updating appInfo');
    p.appInfo.productName="ravel";
    p.appInfo.sanitizedProductName="ravel";
    p.appInfo.productFilename="ravel";
    if (p.platform.name==="windows")
      p.platformSpecificBuildOptions.icon="apps/minsky-electron/src/assets/RavelLogo.ico"
  }
}
