import { promises as fsPromises } from 'fs';
import * as path from 'path';

abstract class HelpFilesManager {
  private static topicNodeMap: Record<string, string> = {};

  public static getHelpFileForType(type: string): string {
    if (type in this.topicNodeMap) {
      console.log("help for ",type," is in ",this.topicNodeMap[type]);
      return this.topicNodeMap[type];
    }
    console.log("no help for ",type);
    return null;
  }

  public static async initialize(directory: string) {
    await this.processFileOrDirectory(directory);
  }

  private static async processFileOrDirectory(fName: string) {
    let stat = null;
    try {
      stat = await fsPromises.lstat(fName);
    } catch (error) {
      console.warn(error);
    }

    if (!stat) {
      return;
    }

    if (stat.isFile()) {
      if (fName.endsWith('.html')) {
        await this.processDocumentFile(fName);
      }
    } else {
      const files = await fsPromises.readdir(fName);
      const promises = [];
      files.forEach(async (fileName) => {
        promises.push(this.processFileOrDirectory(fName + '/' + fileName));
      });
      await Promise.all(promises);
    }
  }

  private static async processDocumentFile(fName: string) {
    const buffer = await fsPromises.readFile(fName);
    if (buffer) {
      const contents = buffer.toString();
      const matches = contents.matchAll(/<A[ \t]+NAME="([^"]*)"/g);
      console.log("processing ",fName);
      for (const match of matches) {
        console.log("adding topicNodeMap for ",match[1]);
        this.topicNodeMap[match[1]] = path.basename(fName);
      }
    }
  }
}

export { HelpFilesManager };
