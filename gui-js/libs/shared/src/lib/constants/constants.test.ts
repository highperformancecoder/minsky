import {describe, expect, test} from '@jest/globals';
import {
  numLinkGroupColours
 } from '@minsky/shared';
import fs from 'fs';

let cwd=process.cwd();
if (cwd.endsWith('shared'))
  var assets='../../apps/minsky-electron/src/assets';
else if (cwd.endsWith('gui-js'))
  var assets='apps/minsky-electron/src/assets';
else {
  console.log(`cwd=${cwd}`);
  console.log("I don't know where the assets directory is, sorry");
  process.exitCode=1;
}
  

describe('Constants tests', ()=>{
  // check that sufficient icons are part of assets
  test('numLinkGroupColours',async ()=>{
    for (let i=0; i<numLinkGroupColours; ++i)
      expect(()=>{fs.statSync(`${assets}/linkGroup${i}.png`)}).not.toThrow();
  });
});
 
