
INSERT INTO `gene` (`id`, `hgnc_id`, `symbol`, `name`, `type`, `ensembl_id`) VALUES
(22713,12692,'VIM','vimentin','protein-coding gene',NULL);

INSERT INTO `gene_transcript` (`id`, `gene_id`, `name`, `source`, `chromosome`, `start_coding`, `end_coding`, `strand`) VALUES 
(85651,22713,'ENST00000224237','ensembl','10',17271422,17279270,'+'),
(85652,22713,'CCDS7120.1','ccds','10',17271422,17279270,'+'),
(85653,22713,'ENST00000544301','ensembl','10',17271422,17279270,'+');

INSERT INTO `gene_exon` (`transcript_id`, `start`, `end`) VALUES
(85651,17271277,17271984),
(85651,17272649,17272709),
(85651,17275586,17275681),
(85651,17275769,17275930),
(85651,17276692,17276817),
(85651,17277168,17277388),
(85651,17277845,17277888),
(85651,17278293,17278378),
(85651,17279229,17279592),
(85652,17271422,17271984),
(85652,17272649,17272709),
(85652,17275586,17275681),
(85652,17275769,17275930),
(85652,17276692,17276817),
(85652,17277168,17277388),
(85652,17277845,17277888),
(85652,17278293,17278378),
(85652,17279229,17279270),
(85653,17270258,17270523),
(85653,17271275,17271984),
(85653,17272649,17272709),
(85653,17275586,17275681),
(85653,17275769,17275930),
(85653,17276692,17276817),
(85653,17277168,17277388),
(85653,17277845,17277888),
(85653,17278293,17278378),
(85653,17279229,17279584);
