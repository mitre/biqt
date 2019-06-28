// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

package org.mitre.biqt;

public class ProviderInfo {

  private String name;
  private String version;
  private String description;
  private String modality;

  /**
   * Creates a new instance of the provider. Intended to be called from native
   * code, and should not be instantiated from Java.
   *
   * @param name The name of the provider
   * @param version The version number of the provider
   * @param description The description of the provider
   * @param modality The modality type of the provider
   */
  private ProviderInfo(String name, String version, String description,
                       String modality) {
    this.name = name;
    this.version = version;
    this.description = description;
    this.modality = modality;
  }

  /**
   * Gets the name for this provider
   *
   * @return The name of this provider as a String
   */
  public String getName() { return this.name; }

  /**
   * Gets the version number of this provider
   *
   * @return The version number as a String
   */
  public String getVersion() { return this.version; }

  /**
   * Gets the description of the provider
   *
   * @return The description of this provider as a String
   */
  public String getDescription() { return this.description; }

  /**
   * Gets the modality of the provider
   *
   * @return The modality of the provider as a String
   */
  public String getModality() { return this.modality; }
}
