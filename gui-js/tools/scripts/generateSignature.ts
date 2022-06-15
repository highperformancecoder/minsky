import { promises } from 'fs';
import { join } from 'path';
import { commandsMapping, green } from './../../libs/shared/src';
const JSON5 = require('json5');

async function generateSignature() {
  const addonDir = '../../../node-addons';

  // eslint-disable-next-line @typescript-eslint/no-var-requires
  let restService = null;
  try {
    restService = require('bindings')(join(addonDir, 'minskyRESTService.node'));
  } catch (error) {
    console.error(error);
  }

  const listSignatures = {};

  const list = JSON5.parse(
    restService.call(commandsMapping.LIST_V2, '')
  ) as string[];

  for (const l of list) {
    const signature = JSON5.parse(
      restService.call(`/minsky${l}/@signature`, '')
    ) as Record<string, unknown>;

    listSignatures[l] = signature;
  }

  await promises.writeFile(
    'signature.json',
    JSON5.stringify(listSignatures, null, 4)
  );

  console.log(
    green('\n 🚀🚀🚀🚀 New Signature File Generated............... 🚀🚀🚀🚀 \n')
  );
}

(async () => {
  await generateSignature().catch(console.error);
  process.exit();
})();
