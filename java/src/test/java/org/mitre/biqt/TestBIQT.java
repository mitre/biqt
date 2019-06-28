package org.mitre.biqt;

import org.json.simple.JSONObject;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import java.util.List;
import java.util.ArrayList;

public class TestBIQT {

  @Test
  public void testGetProviders() {
    BIQT app = new BIQT();
	int foundCount = 0;
    List<ProviderInfo> providers = app.getProviders();
    assertTrue(providers.size() >= 2);

    for (ProviderInfo p : providers) {
	  if (p.getName().equals("BIQTFace") || p.getName().equals("BIQTIris")) {
		  foundCount++;
	  }
    }
    assertEquals(2, foundCount);
  }

  @Test
  public void testRunProvider() {
    BIQT app = new BIQT();
    ArrayList<String> inputFiles = new ArrayList();
    inputFiles.add(System.getenv("BIQT_HOME") + "/../images/iris1.bmp");
    List<JSONObject> results = app.runProvider("BIQTIris", inputFiles);
    assertTrue("Received an empty result set", results.size() > 0);
  }

  @Test
  public void testRunModality() {
    BIQT app = new BIQT();
    ArrayList<String> inputFiles = new ArrayList();
    inputFiles.add(System.getenv("BIQT_HOME") + "/../images/iris1.bmp");
    List<JSONObject> results = app.runModality("iris", inputFiles);
    assertTrue("Received an empty result set", results.size() > 0);
  }
}
