package src.node;

import src.node.fault.Fault;
import src.node.graphics.Graphics;
import src.node.app.App;
import src.node.dtn.contactplan.ContactDtn;
import src.node.dtn.IDtn;
import src.node.com.ICom;

module Node
{
    parameters:
        @display("i=device/satellite");
        string icon = default("satellite");
       
    submodules:
        app: App {
            parameters:
                @display("p=75,50");
        }
        dtn: <default("ContactDtn")> like IDtn {
            parameters:
                @display("p=75,120");
        }
        com: <default("ContactPlanCom")> like ICom {
            parameters:
                @display("p=75,190");
        }
        graphics: Graphics {
            parameters:
                @display("p=150,50");
        }
        fault: Fault {
            parameters:
                @display("p=150,120");
        }

    connections:
        app.gateToDtn <--> dtn.gateToApp;
        dtn.gateToCom <--> com.gateToDtn;
}
