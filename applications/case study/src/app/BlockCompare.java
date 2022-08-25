package app;

import data.PrincipalComponentsAnalysis;
import math.Matrix;
import math.Vector;

public class BlockCompare {

	private static final String usage = 
		"usage: BlockCompare file1 file2\n";

	private static void exitWithError(String msg) {
		if (msg.length() > 0)
			System.err.println(msg);
		System.err.println(usage);
		System.exit(1);
	}	

	private static final String attributes[] = {
		"realBackgroundColor", "realTextColor", "realFontSize",	"realFontWeight"
	};	
	
	public static void main(String[] args) {
		if (args.length < 2)
			exitWithError("not enough arguments");
		
		Matrix blocks1 = null, blocks2 = null;
		try {
			XmlBlockReader reader = new XmlBlockReader(args[0], attributes);
			blocks1 = reader.getData();
			reader = new XmlBlockReader(args[1], attributes);
			blocks2 = reader.getData();
		}
		catch (RuntimeException ex) {
			System.err.println("Error parsing XML input: " + ex.getMessage());
			System.exit(1);
		}

		int dim = blocks1.height();
		
		Vector eigenValues = new Vector(dim);
		Matrix pcVectors1 =
			PrincipalComponentsAnalysis.getComponents(blocks1, eigenValues);
		Matrix pcVectors2 =
			PrincipalComponentsAnalysis.getComponents(blocks2, eigenValues);
		
		for (int i = 1; i <= dim; i++) {
			// compute sPCA for the i principal components with the highest
			// eigenvalues
			double s = PrincipalComponentsAnalysis.similarity(pcVectors1,
					pcVectors2, i);
			System.out.print(Double.toString(s) + ";");
		}
		System.out.println();
	}

}
