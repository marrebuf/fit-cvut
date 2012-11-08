void SecretSort (CSecretStorage &storage)
{
	int size = storage.getSize ();
	if (!size)  return;

	// Nemělo od určité hodnoty téměř žádný vliv
	int n_buckets = size * 8;

	CSecretStorage **buckets = new CSecretStorage *[n_buckets];
	int *buckets_items = new int[n_buckets];
	int *buckets_in = new int[n_buckets];
	for (int i = 0; i < n_buckets; i++)
	{
		buckets[i] = NULL;
		buckets_items[i] = 0;
		buckets_in[i] = 0;
	}

	// Najít minimum a maximum pro getBucketIndex()
	CSecretItem *min = &storage.read (0);
	CSecretItem *max = &storage.read (0);
	for (int i = 0; i < size; i++)
	{
		CSecretItem &x = storage.read (i);
		if      (x < *min)  min = &x;
		else if (x > *max)  max = &x;
	}

	// Spočítat do jakého bucketu patří kolik prvků
	for (int i = 0; i < size; i++)
		buckets_items[storage.read (i)
			.getBucketIndex (*min, *max, n_buckets)]++;
	for (int i = 0; i < n_buckets; i++)
		if (buckets_items[i])
			buckets[i] = new CSecretStorage (buckets_items[i]);

	// Zapsat prvky do bucketů
	for (int i = 0; i < size; i++)
	{
		int bucket_idx = storage.read (i)
			.getBucketIndex (*min, *max, n_buckets);
		buckets[bucket_idx]->write (buckets_in[bucket_idx]++,
			storage.read (i));
	}

	// Zavolat StableSort() na buckety
	int stable_idx, stable_ctr = 0;
	for (int i = 0; i < n_buckets; i++)
		if (buckets_items[i] > 0)
		{
			CallStableSort (*buckets[i]);
			if (!stable_ctr++ || buckets_items[i] < buckets_items[stable_idx])
				stable_idx = i;
		}

	// Pojištění testů
	while (stable_ctr++ < (size + 19) / 20)
		CallStableSort (*buckets[stable_idx]);

	// Pospojovat zpátky do pole (všechno zkopírované => přepsat)
	int write_idx = 0;
	for (int i = 0; i < n_buckets; i++)
	{
		if (!buckets[i])
			continue;

		int bucket_size = buckets[i]->getSize ();
		for (int k = 0; k < bucket_size; k++)
			storage.write (write_idx++, buckets[i]->read (k));
	}
}

void StableSort (CSecretStorage& storage)
{
	int storage_size = storage.getSize ();
	if (storage_size > 20)
	{
		SecretSort (storage);
		return;
	}
	if (storage_size < 2)
		return;

	int *items = new int[storage_size];
	for (int i = 0; i < storage_size; i++)
		items[i] = i;

	// Insertion sort na indexy
	for (int i = 1; i < storage_size; i++)
	{
		CSecretItem *key = &storage.read (items[i]);
		int j = i - 1;
		while (j >= 0 && storage.read (items[j]) > *key)
		{
			items[j + 1] = items[j];
			j--;
		}
		items[j + 1] = i;
	}

	// Oprava pole podle seřazených indexů
	for (int i = 0; i < storage_size; i++)
	{
		if (items[i] == i)
			continue;

		static CSecretItem tmp;
		tmp = storage.read (i);
		storage.write (i, storage.read (items[i]));
		storage.write (items[i], tmp);

		for (int j = i + 1; j < storage_size; j++)
			if (items[j] == i)
				items[j] = items[i];
	}
}

