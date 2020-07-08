using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.IO.Pipes;


namespace SimpleDebugger
{
    class Program
    {
        static void Main(string[] args)
        {
            MapFile mf = new MapFile("../../../../Release/MyOS_1.map");
                mf.ParseFile();
            //return;

            while (true)
            {
                using (NamedPipeClientStream pipeClient =
                    new NamedPipeClientStream(".", "MyOS_PIPE", PipeDirection.InOut))
                {

                    // Connect to the pipe or wait until the pipe is available.
                    Console.Write("Attempting to connect to pipe...");
                    pipeClient.Connect();

                    Console.WriteLine("Connected to pipe.");
                    Console.WriteLine("There are currently {0} pipe server instances open.",
                       pipeClient.NumberOfServerInstances);
                    using (StreamReader sr = new StreamReader(pipeClient))
                    {
                        // Display the read text to the console
                        string temp;
                        bool awaitingModuleName = false;
                        bool canMap = false;
                        while ((temp = sr.ReadLine()) != null)
                        {
                            // If we see "Hello world!" we know the kernel restarted, and we should reload all map files
                            if(temp.Contains("Hello world!"))
                            {
                                Console.WriteLine("Reloading map files");
                                mf.ParseFile();
                                awaitingModuleName = false;
                                continue;
                            }
                                               
                            if(temp == "Stack trace:")
                            {
                                awaitingModuleName = true;
                                continue;
                            }
                                                        
                            if(awaitingModuleName)
                            {
                                awaitingModuleName = false;

                                // module name will be stored in temp
                                if (temp == "KERNEL PROCESS")
                                {
                                    System.Console.WriteLine(temp);
                                    System.Console.WriteLine("Using map file " + mf.filename + "\n");
                                    canMap = true;
                                    continue;
                                }
                                else
                                {
                                    System.Console.WriteLine("Don't know where to find map file for " + temp);
                                    canMap = false;
                                }
                            }

                            if (temp.Contains("0x") && canMap)
                            {
                                Console.WriteLine(mf.FindFunction(temp));
                            }
                            else
                            {
                                // if address was in kernel space
                                if (temp.Contains("0x") && Convert.ToUInt32(temp, 16) >= 0xC0000000)
                                {
                                    Console.WriteLine(mf.FindFunction(temp));
                                }
                                else
                                    Console.WriteLine("{0}", temp);
                            }
                        }
                    }
                }
                Console.Write("\nPipe closed, Reopening...");
            }
            //Console.ReadLine();
        }
    }
}
