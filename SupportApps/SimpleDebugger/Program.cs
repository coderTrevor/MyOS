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
                    while ((temp = sr.ReadLine()) != null)
                    {
                        if(temp.Contains("0x"))
                        {
                            Console.WriteLine(mf.FindFunction(temp));
                        }
                        else
                            Console.WriteLine("{0}", temp);
                    }
                }
            }
            Console.Write("\nPipe closed, Reopening...");
        }
        //Console.ReadLine();
    }
}
}
