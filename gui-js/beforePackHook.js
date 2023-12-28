let glob=require('glob');
exports.default = async function(context) {
  let p=context.packager;
  // if libravel is present in dynamic_libraries, this is a ravel build.
  glob.glob(p.info.projectDir+"/dynamic_libraries/libravel.*",function(er,files)
       {
         if (files.length>0)
         {
           p.appInfo.productName="ravel";
           p.appInfo.sanitizedProductName="ravel";
           p.appInfo.productFilename="ravel";
           if (p.platform.name==="windows")
             p.platformSpecificBuildOptions.icon="apps/minsky-electron/src/assets/RavelLogo.ico"
         }
       });
}
