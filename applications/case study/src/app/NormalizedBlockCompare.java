package app;

import math.Vector;
import data.IVectorCompare;

public class NormalizedBlockCompare implements IVectorCompare {

	private int algo;
	private double sqrt2;
	
	public NormalizedBlockCompare(int algo) {
		this.algo = algo;
		this.sqrt2 = Math.sqrt(2.0);
	}

	public int compare(Vector v1, Vector v2) {
		double w1 = 0.0, w2 = 0.0;

		switch (algo) {
		default:
		case 0:  // top to bottom, then left to right
			w1 = 10.0 * v1.get(1) + v1.get(0);
			w2 = 10.0 * v2.get(1) + v2.get(0);
			break;
		case 1:  // left to right, then top to bottom
			w1 = 10.0 * v1.get(0) + v1.get(1);
			w2 = 10.0 * v2.get(0) + v2.get(1);
			break;
		case 2:  // top to bottom, then distance to vertical middle axis
			w1 = 10.0 * v1.get(1) + 2.0 * Math.abs(v1.get(0) - 0.5);
			w2 = 10.0 * v2.get(1) + 2.0 * Math.abs(v2.get(0) - 0.5);
			break;
		case 3:  // left top to bottom right, then distance to diagonal
			double l1sqr, l2sqr, x, d;

			l1sqr = Math.pow(v1.get(0), 2.0) + Math.pow(v1.get(1), 2.0);
			l2sqr = Math.pow(1.0 - v1.get(0), 2.0) +
			        Math.pow(1.0 - v1.get(1), 2.0);
			x = l1sqr / 4.0 - l2sqr / 4.0 - 0.5;
			d = Math.sqrt(l1sqr - 2.0 / (x * x));
			w1 = 10.0 * x + (2.0 * d) / sqrt2;

			l1sqr = Math.pow(v2.get(0), 2.0) + Math.pow(v2.get(1), 2.0);
			l2sqr = Math.pow(1.0 - v2.get(0), 2.0) +
			        Math.pow(1.0 - v2.get(1), 2.0);
			x = l1sqr / 4.0 - l2sqr / 4.0 - 0.5;
			d = Math.sqrt(l1sqr - 2.0 / (x * x));
			w2 = 10.0 * x + (2.0 * d) / sqrt2;
			break;
		}

		double diff = w1 - w2;
		if (Math.abs(diff) < 0.0001)
			return 0;
		else if (diff < 0)
			return -1;
		else
			return 1;
	}

}
