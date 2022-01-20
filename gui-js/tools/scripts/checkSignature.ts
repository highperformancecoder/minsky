import { promises } from 'fs';
import { join } from 'path';
import { commandsMapping, green, red } from './../../libs/shared/src';
const JSON5 = require('json5');

async function checkSignature() {
  const addonDir = '../../../node-addons';

  // eslint-disable-next-line @typescript-eslint/no-var-requires
  let restService = null;
  try {
    restService = require('bindings')(join(addonDir, 'minskyRESTService.node'));
  } catch (error) {
    console.error(error);
  }

  const list = JSON5.parse(
    restService.call(commandsMapping.LIST_V2, '')
  ) as string[];

  const _signature = JSON5.parse(
    (await promises.readFile('signature.json')).toString()
  );

  for (const l of list) {
    const signature = JSON5.parse(
      restService.call(`/minsky${l}/@signature`, '')
    ) as Record<string, unknown>;

    if (
      !_signature[l] ||
      JSON5.stringify(_signature[l]) !== JSON5.stringify(signature)
    ) {
      console.log(
        red(
          `\n 🚀🚀🚀🚀 Signature changed for /minsky${l} ................ 🚀🚀🚀🚀 \n`
        )
      );

      console.log(
        green(`\n    Before: ${JSON.stringify(_signature[l])}     \n`)
      );
      console.log(red(`\n    After: ${JSON.stringify(signature)}    \n`));
      return;
    }
  }

  console.log(
    green('\n 🚀🚀🚀🚀 No Change In Signature................ 🚀🚀🚀🚀 \n')
  );
}

(async () => {
  await checkSignature().catch(console.error);
})();
