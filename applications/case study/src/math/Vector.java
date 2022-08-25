package math;

public class Vector {

	protected double[] data;


	public Vector(int dim) {
		this.data = new double[dim];
	}
	
	public Vector(double[] data) {
		this.data = data;
	}
	
	public Vector(int dim, double value) {
		this.data = new double[dim];
		java.util.Arrays.fill(this.data, value);
	}
	
	public Vector(Vector other) {
		int length = other.data.length;
		this.data = new double[length];
		System.arraycopy(other.data, 0, this.data, 0, length);
	}


	public int size() {
		return data.length;
	}
	
	public void clear() {
		java.util.Arrays.fill(data, 0.0);
	}

	public double get(int i) {
		return data[i];
	}
	
	public void set(int i, double value) {
		data[i] = value;
	}


	public void add(Vector v) {
		int d = data.length;
		assert (v.data.length == d);
		
		for (int i = 0; i < d; i++)
			data[i] += v.data[i];
	}
	
	public static Vector add(Vector v1, Vector v2) {
		// v1 + v2
		Vector result = new Vector(v1);
		result.add(v2);
		return result;		
	}
	
	public void sub(Vector v) {
		int d = data.length;
		assert (v.data.length == d);
		
		for (int i = 0; i < d; i++)
			data[i] -= v.data[i];
	}
	
	public static Vector sub(Vector v1, Vector v2) {
		// v1 - v2
		Vector result = new Vector(v1);
		result.sub(v2);
		return result;
	}
	
	public void mult(double x) {
		for (int i = 0; i < data.length; i++)
			data[i] *= x;
	}
	
	public static Vector mult(double x, Vector v) {
		// x * v
		Vector result = new Vector(v);
		result.mult(x);
		return result;
	}
	
	// scalar product
	public double mult(Vector v) {
		double x = 0.0;
		for (int i = 0; i < data.length; i++)
			x += data[i] * v.data[i];
		return x;
	}

	public double length() {
		double x = 0.0;
		for (int i = 0; i < data.length; i++)
			x += data[i] * data[i];
		return Math.sqrt(x);
	}

	public void normalize() {
		double l = length();
		if (l == 0)
			return;
		for (int i = 0; i < data.length; i++)
			data[i] /= l;
	}

	
	public double euclideanDist(Vector v) {
		return Math.sqrt(euclideanDistSqr(v));
	}

	public double euclideanDistSqr(Vector v) {
		int n = v.data.length;
		assert (n == this.data.length);

		double d = 0.0;
		for (int i = 0; i < n; i++) {
			double x = data[i] - v.data[i];
			d += x * x;
		}

		return d;
	}

}
