using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SimpleDebugger
{
    class MapFile
    {
        public MapFile(string fileName)
        {
            filename = fileName;
        }

        public void ParseFile()
        {
            string[] fileLines = File.ReadAllLines(filename);

            bool parsing = false;
            foreach (string line in fileLines)
            {
                if(parsing)
                {
                    if (line.Length == 0)
                        continue;

                    if(line.Contains(" entry point at"))
                    {
                        parsing = false;
                        continue;
                    }

                    // Skip the section:memory part and jump to the public name
                    string lineSub = line.Substring(21);
                    //System.Console.Write(lineSub + "\n");

                    string[] lineSplit = lineSub.Split(' ');

                    //int spaces = 0;
                    int stage = 0;  // stage 0 = public name; stage 1 = address; stage 2 = object name
                    foreach (string split in lineSplit)
                    {
                        if (split.Length == 0)
                            continue;

                        if (split == "f" || split == "i")
                            continue;

                        switch(stage)
                        {
                            case 0:
                                publicNames.Add(split);
                                break;
                            case 1:
                                addresses.Add(Convert.ToUInt32(split, 16));
                                break;
                            case 2:
                                objectNames.Add(split);
                                break;
                            default:
                                System.Console.Write("Reached stage 3\n");
                                break;
                        }

                        ++stage;
                        /*for (int i = 0; i < spaces; ++i)
                          System.Console.Write(" ");

                        spaces++;
                        System.Console.Write(split + "\n");*/                                                
                    }
                }
                else
                {
                    // Not parsing yet, find the first line
                    if (line.Contains("  Address"))
                        parsing = true;

                    //System.Console.Write(line + "\n");
                }
                
            }
        }
        
        public string FindFunction(string hex)
        {
            UInt32 needle = Convert.ToUInt32(hex, 16);

            int i = 0;
            UInt32 offset = 0;
            // We need to find the index of the address that's equal to the address we're searching,
            // or the index of the highest address that's less than the needle address
            foreach(UInt32 address in addresses)
            {
                //if (needle > address)
                //    continue;

                if(needle == address)
                {
                    // we're done
                    break;
                }

                if(needle < address)
                {
                    // we went too far
                    --i;
                    if (i >= 0)
                        offset = needle - addresses[i];
                    break;
                }

                ++i;
            }

            if(i < 0)
            {
                return "<Address too low>";
            }

            if (offset > 0)
                return publicNames[i] + "+" + offset + "  in  " + objectNames[i];
                              
            return publicNames[i] + "  in  " + objectNames[i];
        }

        public string filename;
        List<string> publicNames = new List<string>();
        List<UInt32> addresses = new List<UInt32>();
        List<string> objectNames = new List<string>();
    }
}
