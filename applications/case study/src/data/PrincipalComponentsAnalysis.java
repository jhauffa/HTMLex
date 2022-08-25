package data;

import math.Matrix;
import math.Vector;

public class PrincipalComponentsAnalysis {

	private static Matrix getMeanAdjustedData(Matrix data) {
		Matrix adjData = new Matrix(data);

		for (int i = 0; i < adjData.height(); i++) {
			// compute mean
			DataSet row = new DataSet(data, i);
			double mean = row.mean();
			
			// subtract mean from each data point
			for (int j = 0; j < adjData.width(); j++)
				adjData.set(i, j, adjData.get(i, j) - mean);
		}

		return adjData;
	}
	
	public static Matrix getComponents(Matrix data, Vector eigenValues) {
		int n = data.height();
		Matrix adjData = getMeanAdjustedData(data);

		DataRecord r = new DataRecord(adjData);
		Matrix cov = r.covarianceMatrix();

		Matrix eigenVectors = new Matrix(n, n);
		cov.eigenSymmetric(eigenVectors, eigenValues);
		return eigenVectors;
	}

	public static double similarity(Matrix pc1, Matrix pc2, int numComponents) {
		double s = 0.0;

		for (int i = 0; i < numComponents; i++) {
			Vector v1 = pc1.getColumn(i);

			for (int j = 0; j < numComponents; j++) {
				Vector v2 = pc2.getColumn(j);

				double l = v1.length() * v2.length();
				if (Math.abs(l) <= Double.MIN_VALUE)
					throw new RuntimeException("PC vector of length 0!");

				s += Math.pow(v1.mult(v2) / l, 2.0);
			}
		}

		return s / numComponents;
	}
	
}
