package math;

public class Matrix {

	protected double[][] data;


	public Matrix(int height, int width) {
		this.data = new double[height][];
		for (int i = 0; i < height; i++)
			this.data[i] = new double[width];
	}
	
	public Matrix(double[][] data) {
		this.data = data;
	}
	
	public Matrix(Matrix other) {
		int height = other.data.length;
		int width = other.data[0].length;
		this.data = new double[height][];
		for (int i = 0; i < height; i++) {
			this.data[i] = new double[width];
			System.arraycopy(other.data[i], 0, this.data[i], 0, width);
		}
	}
	

	public int width() {
		return data[0].length;
	}
	
	public int height() {
		return data.length;
	}
	
	public void clear() {
		for (int i = 0; i < data.length; i++)
			java.util.Arrays.fill(data[i], 0.0);
	}

	public double get(int i, int j) {
		return data[i][j];
	}

	public void set(int i, int j, double value) {
		data[i][j] = value;
	}
	
	public Vector getColumn(int c) {
		int h = data.length;
		Vector v = new Vector(h);
		for (int i = 0; i < h; i++)
			v.set(i, data[i][c]);
		return v;
	}
	
	public void setColumn(int c, Vector v) {
		for (int i = 0; i < data.length; i++)
			data[i][c] = v.get(i);
	}
	
	public void swapColumns(int c1, int c2) {
		for (int i = 0; i < data.length; i++) {
			double tmp = data[i][c2];
			data[i][c2] = data[i][c1];
			data[i][c1] = tmp;
		}
	}
	
	public void appendRight(double a[][]) {
		int height = data.length;
		if (a.length != height)
			throw new RuntimeException("height mismatch");

		double newData[][] = new double[height][];
		for (int i = 0; i < height; i++) {
			int width = data[i].length;
			newData[i] = new double[width + a[i].length];
			System.arraycopy(data[i], 0, newData[i], 0, width);
			System.arraycopy(a[i], 0, newData[i], width, a[i].length);
		}
		data = newData;
	}

	
	public double frobeniusNorm() {
		double norm = 0.0;
		for (int i = 0; i < data.length; i++)
			for (int j = 0; j < data[i].length; j++)
				norm += data[i][j] * data[i][j];
		return Math.sqrt(norm);
	}

	public void transpose() {
		for (int i = 0; i < data.length; i++)
			for (int j = i + 1; j < data[0].length; j++) {
				double tmp = data[i][j];
				data[i][j] = data[j][i];
				data[j][i] = tmp;
			}
	}
	
	/**
	 * Scale values, so that smallest value becomes 0.0 and largest value
	 * becomes 1.0. 
	 */
	public void normalize() {
		double min = Double.POSITIVE_INFINITY;
		double max = Double.NEGATIVE_INFINITY;
		for (int i = 0; i < data.length; i++) {
			for (int j = 0; j < data[i].length; j++) {
				double v = data[i][j];
				if (v < min)
					min = v;
				if (v > max)
					max = v;
			}
		}
		
		for (int i = 0; i < data.length; i++)
			for (int j = 0; j < data[i].length; j++)
				data[i][j] = (data[i][j] - min) / (max - min);
	}
	
	/**
	 * Performs simple LU-decomposition without permutation; not numerically
	 * stable.
	 * @param m  matrix to decompose
	 * @return   (L - I) + U
	 */
	private Matrix luDecompose() {
		int n = data.length;
		Matrix lu = new Matrix(this);

		for (int i = 0; i < n; i++) {
			for (int j = i; j < n; j++)
				for (int k = 0; k < i; k++)
					lu.data[i][j] -= lu.data[i][k] * lu.data[k][j];
			
			for (int j = i + 1; j < n; j++) {
				for (int k = 0; k < i; k++)
					lu.data[j][i] -= lu.data[j][k] * lu.data[k][i];
				lu.data[j][i] /= lu.data[i][i];
			}
		}
		
		return lu;
	}
	
	public double determinant() {
		int n = data.length;
		assert (n == data[0].length);

		// perform LU decomposition; return product of diagonal elements of U
		double det = 1.0;
		Matrix lu = luDecompose();
		for (int i = 0; i < n; i++)
			det = det * lu.data[i][i];

		return det;
	}

	public Matrix invert() {
		// Compute each column of X, the inverse matrix of M, by solving the
		// equation M * Xi = Ii, where Xi is the i-th column of the inverse
		// matrix and Ii the i-th column of the identity matrix. The equation is
		// solved using LU-decomposition.
		int n = data.length;
		assert (n == data[0].length);

		Matrix inv = new Matrix(n, n);
		Matrix lu = luDecompose();

		// compute each column of the inverse matrix
		for (int i = 0; i < n; i++) {
			// forward substitution: solving L * y = Ii
			double[] y = new double[n];  // all elements are initialized to 0.0
			for (int j = 0; j < n; j++) {
				if (j == i)
					y[j] = 1.0;
				for (int k = 0; k < j; k++)
					y[j] -= lu.data[j][k] * y[k];
			}
			
			// backward substitution: solving U * Xi = y
			for (int j = n - 1; j >= 0; j--) {
				for (int k = j + 1; k < n; k++)
					y[j] -= lu.data[j][k] * inv.data[k][i];
				inv.data[j][i] = y[j] / lu.data[j][j];
			}
		}	
			
		return inv;
	}

	
	private static final int MAX_ROTATIONS = 50;
	
	// Compute the eigen-decomposition of a symmetric matrix.
	public void eigenSymmetric(Matrix vectors, Vector values) {
		int n = data.length;
		if (n != data[0].length)
			throw new RuntimeException("matrix not square");
		if ((vectors.height() != n) || (vectors.width() != n) ||
			(values.size() != n))
			throw new RuntimeException("invalid size of output vector/matrix");
		
		int i;
		for (i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (i == j)
					vectors.set(i, j, 1.0);
				else
					vectors.set(i, j, 0.0);
			}
		}

        Matrix a = new Matrix(this);
        Vector b = new Vector(n);
        Vector z = new Vector(n);
        for (i = 0; i < n; i++) {
        	double tmp = a.get(i, i);
        	b.set(i, tmp);
        	values.set(i, tmp);
        }

        for (i = 0; i < MAX_ROTATIONS; i++) {
            double sum = 0.0;
            for (int j = 0; j < (n - 1); j++)
                for (int k = j + 1; k < n; k++)
                    sum += Math.abs(a.get(j, k));
            if (sum == 0.0)
                break;

            double threshold = 0.0;
            if (i < 3)
                threshold = 0.2 * sum / (n * n);

            for (int j = 0; j < (n - 1); j++) {
                for (int k = j + 1; k < n; k++) {
                    double g = 100.0 * Math.abs(a.get(j, k));
                    if ((i > 3) &&
                    	((Math.abs(values.get(j)) + g) == Math.abs(values.get(j))) &&
                        ((Math.abs(values.get(k)) + g) == Math.abs(values.get(k)))) {
                        a.set(j, k, 0.0);
                    }
                    else if (Math.abs(a.get(j, k)) > threshold) {
                        double h = values.get(k) - values.get(j);
                        double t;
                        if ((Math.abs(h) + g) == Math.abs(h)) {
                            t = a.get(j, k) / h;
                        }
                        else {
                            double theta = 0.5 * h / a.get(j, k);
                            t = 1.0 / (Math.abs(theta) +
                            		   Math.sqrt(1.0 + theta * theta));
                            if (theta < 0.0)
                                t = -t;
                        }

                        double c = 1.0 / Math.sqrt(1.0 + t * t);
                        double s = t * c;
                        double tau = s / (1.0 + c);
                        h = t * a.get(j, k);
                        z.set(j, z.get(j) - h);
                        z.set(k, z.get(k) + h);
                        values.set(j, values.get(j) - h);
                        values.set(k, values.get(k) + h);
                        a.set(j, k, 0.0);

                        for (int l = 0; l <= (j - 1); l++) {
                            g = a.get(l, j);
                            h = a.get(l, k);
                            a.set(l, j, g - s * (h + g * tau));
                            a.set(l, k, h + s * (g - h * tau));
                        }
                        for (int l = j + 1; l <= (k - 1); l++) {
                            g = a.get(j, l);
                            h = a.get(l, k);
                            a.set(j, l, g - s * (h + g * tau));
                            a.set(l, k, h + s * (g - h * tau));
                        }
                        for (int l = k + 1; l < n; l++) {
                            g = a.get(j, l);
                            h = a.get(k, l);
                            a.set(j, l, g - s * (h + g * tau));
                            a.set(k, l, h + s * (g - h * tau));
                        }
                        for (int l = 0; l < n; l++) {
                            g = vectors.get(l, j);
                            h = vectors.get(l, k);
                            vectors.set(l, j, g - s * (h + g * tau));
                            vectors.set(l, k, h + s * (g - h * tau));
                        }
                    }
                }
            }
        
            for (int j = 0; j < n; j++) {
            	b.set(j, b.get(j) + z.get(j));
            	values.set(j, b.get(j));
            	z.set(j, 0.0);
            }
        }

        if (i >= MAX_ROTATIONS) {
            throw new RuntimeException("Matrix did not converge after maximum" +
            		                   " number of rotations.");
        }

        // sort by descending eigenvalues
        for (int j = 0; j < (n - 1); j++) {
            int maxIdx = j;
            for (int k = j + 1; k < n; k++)
                if (values.get(k) > values.get(maxIdx))
                    maxIdx = k;
            if (maxIdx != j) {
                double tmp = values.get(j);
                values.set(j, values.get(maxIdx));
                values.set(maxIdx, tmp);
                vectors.swapColumns(j, maxIdx);
            }
        }
	}


	/**
	 * component-wise subtraction (m1 - m2)
	 */
	public static Matrix sub(Matrix m1, Matrix m2) {
		Matrix diff = new Matrix(m1);
		diff.sub(m2);
		return diff;
	}

	public void sub(Matrix m) {
		int h = data.length;
		int w = data[0].length;
		assert (h == m.data.length);
		assert (w == m.data[0].length);
		
		for (int i = 0; i < h; i++)
			for (int j = 0; j < w; j++)
				data[i][j] -= m.data[i][j];
	}
	
	public static double mult(Vector v1, Matrix m, Vector v2) {
		// transpose(v1) * m * v2
		assert (v1.size() == m.data.length);
		assert (v2.size() == m.data[0].length);
		
		double product = 0.0;
		for (int i = 0; i < v1.size(); i++) {
			double p = 0.0;
			for (int j = 0; j < v2.size(); j++)
				p += v2.get(j) * m.data[i][j];
			product += v1.get(i) * p;
		}
		
		return product;
	}

	public static Matrix mult(Matrix m1, Matrix m2) {
		int n = m1.data[0].length;
		assert (n == m2.data.length);  // m1.height == m2.width
		
		int h = m1.data.length;
		int w = m2.data[0].length;
		Matrix product = new Matrix(h, w);
		
		for (int i = 0; i < h; i++)
			for (int j = 0; j < w; j++)
				for (int k = 0; k < n; k++)
					product.data[i][j] += m1.data[i][k] * m2.data[k][j];
		
		return product;
	}

	public Vector mult(Vector v) {
		// m = m * v
		int w = data[0].length;
		assert(v.data.length == w);  // vector height == matrix width
		Vector vv = new Vector(data.length);
		for (int i = 0; i < data.length; i++) {
			double sum = 0.0;
			for (int j = 0; j < w; j++)
				sum += data[i][j] * v.get(j); 
			vv.set(i, sum);
		}
		return vv;
	}


	public static Matrix identity(int n) {
		Matrix m = new Matrix(n, n);
		for (int i = 0; i < n; i++)
			m.set(i, i, 1.0);
		return m;
	}
	
}
